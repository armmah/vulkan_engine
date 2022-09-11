#include "pch.h"
#include "Scene.h"
#include "VkMesh.h"
#include "Mesh.h"
#include "Material.h"
#include "VkTypes/VkTexture.h"
#include "VkTypes/VkShader.h"

#include "Presentation/Device.h"
#include "PresentationTarget.h"
#include "EngineCore/StagingBufferPool.h"

#include "FileManager/FileIO.h"
#include "FileManager/Directories.h"
#include "Profiling/ProfileMarker.h"

#include "OpenFBX/ofbx.h"

Scene::~Scene() {}

Scene::Scene(const Presentation::Device* device, Presentation::PresentationTarget* target)
	: m_presentationDevice(device), m_presentationTarget(target) { }

bool Scene::load(VkDescriptorPool descPool)
{
	ProfileMarker _("Scene::load");

	//Directories::getWorkingScene();
	auto path = Directories::getWorkingModel();
	tryLoadFromFile(path.value, descPool);

	printf("Initialized the scene with (renderers = %zi), (meshes = %zi), (textures = %zi), (materials = %zi).\n", m_renderers.size(), m_meshes.size(), m_textures.size(), m_materials.size());
	return true;
}

void Scene::release(VkDevice device, const VmaAllocator& allocator)
{
	for (auto& mesh : m_meshes)
	{
		mesh.clear();
	}
	m_meshes.clear();

	for (auto& gmesh : m_graphicsMeshes)
	{
		gmesh.release(allocator);
	}
	m_graphicsMeshes.clear();

	for (auto& tex : m_textures)
	{
		tex->release(device);
		tex.release();
	}
	m_textures.clear();

	for (auto& mat : m_graphicsMaterials)
	{
		mat->release(device);
		mat.release();
	}
	m_graphicsMaterials.clear();

	m_renderers.clear();
	m_materials.clear();
	m_rendererIDs.clear();
}

const std::vector<Mesh>& Scene::getMeshes() const { return m_meshes; }
const std::vector<Material>& Scene::getMaterials() const { return m_materials; }
const std::vector<Renderer>& Scene::getRendererIDs() const { return m_rendererIDs; }
const std::vector<VkMesh>& Scene::getGraphicsMeshes() const { return m_graphicsMeshes; }
const std::vector<VkMeshRenderer>& Scene::getRenderers() const { return m_renderers; }

namespace tinyobj
{
	struct IndexHash
	{
		static constexpr size_t cantor(size_t a, size_t b) { return (a + b + 1) * (a + b) / 2 + b; }
		static constexpr size_t cantor(int a, int b, int c) { return cantor(a, cantor(b, c)); }

		size_t operator()(const tinyobj::index_t& k)const
		{
			return cantor(k.vertex_index, k.normal_index, k.texcoord_index);
		}
	};

	struct IndexComparison
	{
		bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b)const
		{
			return a.vertex_index == b.vertex_index && a.normal_index == b.normal_index && a.texcoord_index == b.texcoord_index;
		}
	};
}

template <typename T>
void fillArrayWithDefaultValue(std::vector<T>& dst, size_t offset, size_t count)
{
	for (int i = offset; i < std::min(dst.size(), count); i++)
	{
		dst[i] = T();
	}
}

template <typename F, typename T>
void reinterpretCopy(std::vector<F>& src, std::vector<T>& dst)
{
	F* r_src = (F*)src.data();
	reinterpretCopy(src.data(), src.size(), dst);
}

template <typename F, typename T>
void reinterpretCopy(const F* r_src, size_t count_src, std::vector<T>& dst)
{
	T* r_dst = (T*)dst.data();

	auto byteSize_src = count_src * sizeof(F);
	auto byteSize_dst = dst.size() * sizeof(T);
	auto minSize = byteSize_dst < byteSize_src ? byteSize_dst : byteSize_src;

	// Copy the contents of src to dst until dst is filled.
	memcpy(r_dst, r_src, minSize);

	// Fill the rest, if necessary.
	//fillArrayWithDefaultValue(dst, minSize / sizeof(T), dst.size());
	for (int i = minSize / sizeof(T); i < dst.size(); i++)
	{
		dst[i] = T();
	}
}

template <typename F, typename T>
T reinterpretAt(std::vector<F>& src, size_t srcIndex)
{
	srcIndex *= (sizeof(T) / sizeof(F));
	return *reinterpret_cast<T*>(&src[srcIndex]);
}

template <typename F, typename T>
T reinterpretAt_orFallback(std::vector<F>& src, size_t srcIndex)
{
	return src.size() > srcIndex ? reinterpretAt<F, T>(src, srcIndex) : T{};
}

void minVector(glm::vec3& min, const glm::vec3& point)
{
	min.x = std::min(min.x, point.x);
	min.y = std::min(min.y, point.y);
	min.z = std::min(min.z, point.z);
}

void maxVector(glm::vec3& max, const glm::vec3& point)
{
	max.x = std::max(max.x, point.x);
	max.y = std::max(max.y, point.y);
	max.z = std::max(max.z, point.z);
}

bool Scene::loadFBX_Implementation(std::vector<Mesh>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, const std::string& path, const std::string& name)
{
	auto fbxPath = Path(path + name + ".fbx");
	auto charCollection = FileIO::readFile(fbxPath);

	const ofbx::u8* ptr = reinterpret_cast<ofbx::u8*>(charCollection.data());
	const auto* scene = ofbx::load(ptr, charCollection.size(), static_cast<ofbx::u64>(ofbx::LoadFlags::IGNORE_BLEND_SHAPES));
	if (!scene) return false;

	for (size_t meshID = 0; meshID < scene->getMeshCount(); meshID++)
	{
		auto* mesh = scene->getMesh(static_cast<int>(meshID));
		auto* geom = mesh->getGeometry();

		auto* mVerts = geom->getVertices();
		auto mVertCount = geom->getVertexCount();

		auto* mTexcoords = geom->getUVs(0);
		auto* mNorms = geom->getNormals();

		std::vector<MeshDescriptor::TVertexPosition> vertices(mVertCount);
		std::vector<MeshDescriptor::TVertexNormal> normals(mVertCount);
		std::vector<MeshDescriptor::TVertexUV> uvs(mVertCount);
		std::vector<MeshDescriptor::TVertexColor> colors;

		for(int vi = 0; vi < mVertCount; vi++)
		{
			vertices[vi] = glm::vec3(
				static_cast<float>(mVerts[vi].x),
				static_cast<float>(mVerts[vi].y),
				static_cast<float>(mVerts[vi].z)
			);

			uvs[vi] = glm::vec2(
				static_cast<float>(mTexcoords[vi].x),
				static_cast<float>(mTexcoords[vi].y)
			);

			normals[vi] = glm::vec3(
				static_cast<float>(mNorms[vi].x),
				static_cast<float>(mNorms[vi].y),
				static_cast<float>(mNorms[vi].z)
			);
		}

		auto* mIndices = geom->getFaceIndices();
		auto mIndexCount = geom->getIndexCount();
		std::vector<MeshDescriptor::TVertexIndices> indices;
		indices.reserve(mIndexCount / 2 * 3);

		int bufIndex = 0;
		for (int ii = 0; ii + 2 < mIndexCount; ii += 3)
		{
			auto a = mIndices[ii + 0],
				b = mIndices[ii + 1],
				c = mIndices[ii + 2];

			if (c < 0)
			{
				c = ~c;

				indices.push_back( a );
				indices.push_back( b );
				indices.push_back( c );
			}
			else
			{
				auto d = ~mIndices[ii + 3];
				ii += 1;

				indices.push_back( a );
				indices.push_back( b );
				indices.push_back( d );
				indices.push_back( d );
				indices.push_back( b );
				indices.push_back( c );
			}
		}
		std::vector<SubMesh> submeshes(1);
		submeshes[0] = SubMesh(indices);

		meshes.push_back(Mesh(vertices, uvs, normals, colors, submeshes));
		rendererIDs.push_back(Renderer(meshID, { 0 }));
	}

	materials.push_back(Material(0, TextureSource("C:/Git/Vulkan_Engine/Resources/Serialized/sponza_column_a_spec.dds", VK_FORMAT_BC1_RGBA_SRGB_BLOCK, false)));

	return true;
}

bool Scene::loadGLTF_Implementation(std::vector<Mesh>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, const std::string& path, const std::string& name, bool isBinary)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;

	//error and warning output from the load function
	std::string err;
	std::string warn;

	auto loaded = isBinary ? loader.LoadBinaryFromFile(&model, &err, &warn, path) : loader.LoadASCIIFromFile(&model, &err, &warn, path);

#ifdef VERBOSE_INFO_MESSAGE
	if (!warn.empty()) printf("Warning: %s\n", warn.c_str());
#endif
	if (!err.empty()) printf("Error: %s\n", err.c_str());
	if (!loaded)
	{
		return false;
	}

	printf("GLTF loading is not implemented yet.");
	return false;
	/*
	const tinygltf::Scene& scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];
	for (auto nodeIndex : scene.nodes)
	{
		auto& node = model.nodes[nodeIndex];
		auto& mesh = model.meshes[node.mesh];
		
		for (auto& prim : mesh.primitives)
		{
			const auto& accessor = model.accessors[prim.attributes.find("POSITION")->second];
			const auto& posView = model.bufferViews[accessor.bufferView];
			
			model.buffers[posView.buffer];
		}
	}

	printf("loaded %s gltf\n", (isBinary ? "Binary" : "ASCII"));
	return true;
	*/
}

bool Scene::loadOBJ_Implementation(std::vector<Mesh>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, const std::string& path, const std::string& name)
{
	//attrib will contain the vertex arrays of the file
	tinyobj::attrib_t objAttribs;
	//shapes contains the info for each separate object in the file
	std::vector<tinyobj::shape_t> objShapes;
	//materials contains the information about the material of each shape, but we won't use it.
	std::vector<tinyobj::material_t> objMats;

	//error and warning output from the load function
	std::string warn;
	std::string err;

	//load the OBJ file
	auto objPath = (path + name + ".obj");

	{
		ProfileMarker _("	Scene::loadObjImplementation - Load obj from disk");
		tinyobj::LoadObj(&objAttribs, &objShapes, &objMats, &warn, &err, objPath.c_str(), path.c_str());

#ifdef VERBOSE_INFO_MESSAGE
		//make sure to output the warnings to the console, in case there are issues with the file
		if (!warn.empty())
		{
			printf("Warning: %s\n", warn.c_str());
		}
#endif

		//if we have any error, print it to the console, and break the mesh loading.
		//This happens if the file can't be found or is malformed
		if (!err.empty())
		{
			printf("Error: %s\n", err.c_str());
			return false;
		}
	}

	ProfileMarker _("	Scene::loadObjImplementation - Create meshes");
	typedef int MeshID;

	struct SubMeshDesc
	{
		size_t indexCount;
		size_t mappedIndex;
	};

	std::unordered_map<MeshID, std::vector<MaterialID>> meshToMaterialMapping;
	std::unordered_map<MaterialID, SubMeshDesc> uniqueMaterialIDs;
	std::unordered_map<size_t, MeshDescriptor::TVertexIndices> indexMapping;

	std::vector<SubMesh> submeshes;
	std::vector<size_t> materialIDs;
	// MESHES
	meshes.reserve(objShapes.size());
	rendererIDs.reserve(objShapes.size());
	for (MeshID i = 0; i < objShapes.size(); i++)
	{
		indexMapping.clear();

		auto& name = objShapes[i].name;
		auto& mesh = objShapes[i].mesh;
		auto& shapeIndices = mesh.indices;

		// The shape's vertex index here is referencing the big obj vertex buffer,
		// Our vertex buffer for mesh will be much smaller and should be indexed per-mesh, instead of globally.

		if (mesh.material_ids.size() * 3 < shapeIndices.size())
		{
			printf("The material ID array size (3 * %zi) does not match the shapeIndices array size (%zi).\n", mesh.material_ids.size(), shapeIndices.size());
			return false;
		}

		uniqueMaterialIDs.clear();
		size_t materialCount = 0;
		for (auto& id : mesh.material_ids)
		{
			if (uniqueMaterialIDs.count(id) > 0)
			{
				uniqueMaterialIDs[id].indexCount += 1;
				continue;
			}

			SubMeshDesc desc;
			desc.indexCount = 1;
			desc.mappedIndex = materialCount;

			uniqueMaterialIDs[id] = desc;
			materialCount += 1;
		}
		submeshes.clear();
		submeshes.reserve(materialCount);
		materialIDs.clear();
		materialIDs.reserve(materialCount);

		for (auto& kvPair : uniqueMaterialIDs)
		{
			auto materialID = kvPair.first;
			auto submeshDesc = kvPair.second;

			materialIDs.push_back(materialID);
			submeshes.push_back(SubMesh(submeshDesc.indexCount * 3));
		}
		meshToMaterialMapping[i] = materialIDs;

		// Axis-Aligned Bounding Box
		std::vector<glm::vec3> boundsMinMax(materialCount * 2);
		for (size_t k = 0; k < boundsMinMax.size(); k += 2)
		{
			boundsMinMax[k] = glm::vec3(1.f) * std::numeric_limits<float>::max();
			boundsMinMax[k + 1] = glm::vec3(1.f) * std::numeric_limits<float>::lowest();
		}

		/*************************		VERTEX, NORMAL, UV indices -> pick unique and map to [0, N] range		**************************/
		MeshDescriptor::TVertexIndices mappedIndex = 0;
		std::unordered_map<tinyobj::index_t, MeshDescriptor::TVertexIndices, tinyobj::IndexHash, tinyobj::IndexComparison> indmap;
		for (size_t k = 0; k < shapeIndices.size(); k++)
		{
			auto& indices = shapeIndices[k];
			auto curIndex = mappedIndex;

			if (indmap.count(indices) == 0)
			{
				indmap[indices] = mappedIndex;
				mappedIndex += 1;

				if (mappedIndex >= std::numeric_limits< MeshDescriptor::TVertexIndices>::max())
				{
					printf("Index overflow, the mesh format not supported as it has more than %i indices.\n", std::numeric_limits< MeshDescriptor::TVertexIndices>::max());
					break;
				}
			}
			else
			{
				curIndex = indmap[indices];
			}

			auto submeshIndex = uniqueMaterialIDs[mesh.material_ids[k / 3]].mappedIndex;

			// Store min and max vertex components to initialize AABB.
			auto vert = reinterpretAt<float, MeshDescriptor::TVertexPosition>(objAttribs.vertices, indices.vertex_index);
			minVector(boundsMinMax[submeshIndex * 2], vert);
			maxVector(boundsMinMax[submeshIndex * 2 + 1], vert);

			submeshes[submeshIndex].m_indices.push_back(curIndex);
		}

		for (size_t k = 0; k < submeshes.size(); k++)
		{
			submeshes[k].m_bounds = BoundsAABB(boundsMinMax[k * 2], boundsMinMax[k * 2 + 1]);
		}

		std::vector<MeshDescriptor::TVertexPosition> vertices(mappedIndex);
		std::vector<MeshDescriptor::TVertexNormal> normals(mappedIndex);
		std::vector<MeshDescriptor::TVertexUV> uvs(mappedIndex);

		if (objAttribs.normals.size() > objAttribs.vertices.size() ||
			objAttribs.texcoords.size() > objAttribs.vertices.size())
		{
			printf("OBJLoader warning, shape '%s' - The normal count (%i) or uv count (%i) exceeds the vertex count (%i), so they will be discarded.\n",
				name.c_str(), as_uint32(objAttribs.normals.size()), as_uint32(objAttribs.texcoords.size()), as_uint32(objAttribs.vertices.size()));
		}

		/*************************		Copying the vert, norm and uv into their containers, in mapped order		**************************/
		for (auto& kv : indmap)
		{
			auto vi = kv.first.vertex_index;
			auto ni = kv.first.normal_index;
			auto uvi = kv.first.texcoord_index;

			vertices[kv.second] = reinterpretAt<float, MeshDescriptor::TVertexPosition>(objAttribs.vertices, vi);

			normals[kv.second] = reinterpretAt_orFallback<float, MeshDescriptor::TVertexNormal>(objAttribs.normals, ni);
			uvs[kv.second] = reinterpretAt_orFallback<float, MeshDescriptor::TVertexUV>(objAttribs.texcoords, uvi);
		}

		std::vector<MeshDescriptor::TVertexColor> colors;		
		meshes.push_back(Mesh(vertices, uvs, normals, colors, submeshes));
	}

	// MATERIALS
	typedef size_t GlobalBufferMaterialID;
	std::unordered_map<MaterialID, GlobalBufferMaterialID> uniqueMaterials;
	std::vector<size_t> globalBufMaterialIDs;
	materials.reserve(objMats.size());
	for (const auto& kv : meshToMaterialMapping)
	{
		const auto& meshMaterials = kv.second;
		globalBufMaterialIDs.clear();
		globalBufMaterialIDs.reserve(meshMaterials.size());
		for (const auto& materialID : meshMaterials)
		{
			if (materialID < 0)
				continue;

			int index = 0;
			if (uniqueMaterials.count(materialID) > 0)
			{
				globalBufMaterialIDs.push_back(uniqueMaterials[materialID]);
			}
			else
			{
				assert(materialID >= 0 && materialID < objMats.size());
				auto& mat = objMats[materialID];
				auto texPath = path + mat.diffuse_texname;
				if (mat.diffuse_texname.length() > 0 && std::filesystem::exists(texPath))
				{
					uniqueMaterials[materialID] = materials.size();
					globalBufMaterialIDs.push_back(materials.size());
					materials.push_back(Material(0, TextureSource(std::move(texPath))));
				}
				else
				{
					printf("Texture at %s could not be found.\n", texPath.c_str());
				}
			}
		}

		rendererIDs.push_back(Renderer(kv.first, globalBufMaterialIDs));
	}

	return true;
}

namespace boost::serialization
{
	template <typename Ar>
	void serialize(Ar& ar, glm::vec3& v, unsigned _)
	{
		ar& make_nvp("x", v.x)& make_nvp("y", v.y)& make_nvp("z", v.z);
	}

	template <typename Ar>
	void serialize(Ar& ar, glm::vec2& v, unsigned _)
	{
		ar& make_nvp("x", v.x)& make_nvp("y", v.y);
	}
}

bool Scene::tryLoadSupportedFormat(const std::string& path)
{
	auto nameStart = path.find_last_of('/') + 1;
	auto extensionIndex = path.find_last_of('.');
	auto extensionLength = path.length() - extensionIndex;
	auto directory = path.substr(0, nameStart);
	auto name = path.substr(nameStart, path.length() - nameStart - extensionLength);

	/* ================ READ SERIALIZED BINARY =============== */
	if (fileExists(path, ".binary"))
	{
		// Deserialize

		auto stream = std::fstream(path, std::ios::in | std::ios::binary);
		boost::archive::binary_iarchive archive(stream);

		archive >> *this;

		return true;
	}

	/* ================ READ FROM OBJ =============== */
	if (fileExists(path, ".obj"))
	{
		return loadOBJ_Implementation(m_meshes, m_materials, m_rendererIDs, directory, name);
	}

	/* ================ READ FROM FBX =============== */
	if (fileExists(path, ".fbx"))
	{
		return loadFBX_Implementation(m_meshes, m_materials, m_rendererIDs, directory, name);
	}

	/* ================ READ FROM GLTF =============== */
	if (fileExists(path, ".gltf"))
	{
		auto isBinary = (path.substr(extensionIndex + 1, extensionLength) == "glb");
		return loadGLTF_Implementation(m_meshes, m_materials, m_rendererIDs, directory, name, isBinary);
	}

	return false;
}

bool Scene::tryLoadFromFile(const std::string& path, VkDescriptorPool descPool)
{
	if (!tryLoadSupportedFormat(path))
	{
		printf("Could not load scene file '%s', it did not match any of the supported formats.\n", path.c_str());
		return false;
	}

	/* ================= CREATE TEXTURES ================*/
	/* ================= CREATE GRAPHICS MATERIALS ================*/
	std::unordered_map<TextureSource, uint32_t> loadedTextures;
	m_graphicsMaterials.reserve(m_textures.size());
	auto device = m_presentationDevice->getDevice();
	StagingBufferPool stagingBufPool{};
	for (auto& rendererIDs : m_rendererIDs)
	{
		for (auto& matIndex : rendererIDs.materialIDs)
		{
			const auto& mat = m_materials[matIndex];
			const auto& texSrc = mat.getTextureSource();
			if (loadedTextures.count(texSrc) == 0)
			{
				auto size = m_textures.size();
				m_textures.resize(size + 1);
				m_graphicsMaterials.resize(size + 1);
				if (VkTexture2D::tryCreateTexture(m_textures.back(), texSrc, m_presentationDevice, stagingBufPool))
				{
					loadedTextures[texSrc] = as_uint32(size);

					auto* shader = VkShader::findShader(mat.getShaderIdentifier());
					m_presentationTarget->createGraphicsMaterial(m_graphicsMaterials.back(), device, descPool, shader, m_textures.back().get());
				}
				else
				{
					m_textures.resize(size);
					m_graphicsMaterials.resize(size);
					printf("The texture at '%s' could not be loaded.\n", texSrc.path.c_str());
				}
			}
		}
	}

	/* ================= CREATE GRAPHICS MESHES ================*/
	auto defaultMeshDescriptor = Mesh::defaultMeshDescriptor;
	auto vmaAllocator = VkMemoryAllocator::getInstance()->m_allocator;
	const auto count = m_meshes.size();

	m_graphicsMeshes.reserve(count);
	m_renderers.reserve(count);
	for (auto& ids : m_rendererIDs)
	{
		auto& mesh = m_meshes[ids.meshID];

		auto newGraphicsMesh = VkMesh();
		if (!mesh.allocateGraphicsMesh(newGraphicsMesh, vmaAllocator, m_presentationDevice, stagingBufPool) || !mesh.isValid())
			return false;

		if (defaultMeshDescriptor != mesh.getMeshDescriptor())
		{
			printf("Mesh metadata does not match - can not bind to the same pipeline.\n");
			return false;
		}
		m_graphicsMeshes.push_back(std::move(newGraphicsMesh));

		uint32_t submeshIndex = 0;
		for (auto materialIDs : ids.materialIDs)
		{
			const auto& texPath = m_materials[materialIDs].getTextureSource();
			if (!loadedTextures.count(texPath) && loadedTextures[texPath] > 0 && loadedTextures[texPath] < m_graphicsMaterials.size())
			{
				printf("SceneLoader - The material not found for '%zu' mesh.\n", ids.meshID);
				continue;
			}

			m_renderers.push_back(VkMeshRenderer(&m_graphicsMeshes.back(), submeshIndex, mesh.getBounds(submeshIndex), &m_graphicsMaterials[loadedTextures[texPath]]->getMaterialVariant()));
			++submeshIndex;
		}
	}
	stagingBufPool.releaseAllResources();

	return true;
}
