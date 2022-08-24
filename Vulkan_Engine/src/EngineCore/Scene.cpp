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

#include "Profiling/ProfileMarker.h"

Scene::~Scene() {}

Scene::Scene(const Presentation::Device* device, Presentation::PresentationTarget* target)
	: m_presentationDevice(device), m_presentationTarget(target) { }

bool Scene::tryLoadTestScene_1(VkDescriptorPool descPool)
{
	m_meshes.resize(1);
	m_meshes[0] = MAKEUNQ<Mesh>(Mesh::getPrimitiveCube());

	StagingBufferPool stagingBufferPool{};

	m_textures.resize(1);
	if(!VkTexture2D::tryCreateTexture(m_textures[0], "C:/Git/Vulkan_Engine/Resources/vulkan_tutorial_texture.jpg", m_presentationDevice, stagingBufferPool, false))
		return false;

	stagingBufferPool.releaseAllResources();

	m_materials.reserve(1);
	return m_presentationTarget->createMaterial(m_materials[0], m_presentationDevice->getDevice(), descPool, Shader::findShader(0), m_textures[0].get());
}

static inline std::string CRYTEK_SPONZA = "C:/Git/Vulkan_Engine/Resources/sponza.obj";
static inline std::string LIGHTING_SCENE = "C:/Git/Vulkan_Engine/Resources/debrovic_sponza/sponza.obj";
static inline std::string CUBE_GLTF = "C:/Git/Vulkan_Engine/Resources/gltf/Cube_khr.gltf";

bool Scene::load(VkDescriptorPool descPool)
{
	ProfileMarker _("Scene::load");

	tryLoadFromFile(CRYTEK_SPONZA, descPool);

	printf("Initialized the scene with (renderers = %zi), (meshes = %zi), (textures = %zi), (materials = %zi).\n", m_renderers.size(), m_meshes.size(), m_textures.size(), m_materials.size());
	return true;
}

void Scene::release(VkDevice device, const VmaAllocator& allocator)
{
	for (auto& mesh : m_meshes)
	{
		mesh->clear();
		mesh.release();
	}
	m_meshes.clear();

	for (auto& gmesh : m_graphicsMeshes)
	{
		gmesh->release(allocator);
		gmesh.release();
	}
	m_graphicsMeshes.clear();

	for (auto& tex : m_textures)
	{
		tex->release(device);
		tex.release();
	}
	m_textures.clear();

	for (auto& mat : m_materials)
	{
		mat->release(device);
		mat.release();
	}
	m_materials.clear();
}

const std::vector<UNQ<Mesh>>& Scene::getMeshes() const { return m_meshes; }
const std::vector<UNQ<VkMesh>>& Scene::getGraphicsMeshes() const { return m_graphicsMeshes; }

namespace tinyobj
{
	struct IndexHash
	{
		static size_t cantor(size_t a, size_t b) { return (a + b + 1) * (a + b) / 2 + b; }
		static size_t cantor(int a, int b, int c) { return cantor(a, cantor(b, c)); }

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
	T* r_dst = (T*)dst.data();

	auto byteSize_src = src.size() * sizeof(F);
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

bool Scene::loadGLTF_Implementation(std::vector<UNQ<Mesh>>& meshes, std::unordered_map<MeshIndex, SubmeshMaterials>& meshTextureMap, const std::string& path, const std::string& name, bool isBinary)
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
			
			
			prim.attributes.find("");
		}
	}

	printf("loaded %s gltf\n", (isBinary ? "Binary" : "ASCII"));
	return true;
}

bool Scene::loadOBJ_Implementation(std::vector<UNQ<Mesh>>& meshes, std::unordered_map<MeshIndex, SubmeshMaterials>& meshTextureMap, const std::string& path, const std::string& name)
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

	struct SubMeshDesc
	{
		int32_t indexCount;
		uint32_t mappedIndex;
	};

	ProfileMarker _("	Scene::loadObjImplementation - Create meshes");
	std::unordered_map<MaterialID, SubMeshDesc> uniqueMaterialIDs;
	std::unordered_map<int32_t, MeshDescriptor::TVertexIndices> indexMapping;
	for (int32_t i = 0; i < objShapes.size(); i++)
	{
		indexMapping.clear();

		auto& name = objShapes[i].name;
		auto& mesh = objShapes[i].mesh;
		auto& shapeIndices = mesh.indices;

		// The shape's vertex index here is referencing the big obj vertex buffer,
		// Our vertex buffer for mesh will be much smaller and should be indexed per-mesh, instead of globally.
		std::vector<SubMesh> submeshes;

		if (mesh.material_ids.size() * 3 < shapeIndices.size())
		{
			printf("The material ID array size (3 * %zi) does not match the shapeIndices array size (%zi).\n", mesh.material_ids.size(), shapeIndices.size());
			return false;
		}

		uniqueMaterialIDs.clear();
		int32_t materialCount = 0;
		for (int32_t k = 0; k < mesh.material_ids.size(); k++)
		{
			auto id = mesh.material_ids[k];

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
		submeshes.reserve(materialCount);

		auto& shapeMaterialCollection = meshTextureMap[i];
		shapeMaterialCollection.reserve(materialCount);
		for (auto& kvPair : uniqueMaterialIDs)
		{
			auto materialID = kvPair.first;
			auto submeshDesc = kvPair.second;

			submeshes.push_back(SubMesh(submeshDesc.indexCount * 3));

			auto texPath = path + objMats[materialID].diffuse_texname;
			if (objMats[materialID].diffuse_texname.length() > 0 && std::filesystem::exists(texPath))
				shapeMaterialCollection.push_back(texPath);
		}

		// Axis-Aligned Bounding Box
		std::vector<glm::vec3> boundsMinMax(materialCount * 2);
		for (int k = 0; k < boundsMinMax.size(); k += 2)
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
		meshes.push_back(MAKEUNQ<Mesh>(vertices, uvs, normals, colors, submeshes));
	}

	return true;
}

bool Scene::tryLoadSupportedFormat(const std::string& path, std::vector<UNQ<Mesh>>& meshes, std::unordered_map<MeshIndex, SubmeshMaterials>& meshTextureMap)
{
	auto nameStart = path.find_last_of('/') + 1;
	auto extensionIndex = path.find_last_of('.');
	auto extensionLength = path.length() - extensionIndex;
	auto directory = path.substr(0, nameStart);
	auto name = path.substr(nameStart, path.length() - nameStart - extensionLength);

	/* ================ READ FROM OBJ =============== */
	if (fileExists(path, ".obj"))
	{
		return loadOBJ_Implementation(meshes, meshTextureMap, directory, name);
	}

	/* ================ READ FROM GLTF =============== */
	if (fileExists(path, ".gltf"))
	{
		auto isBinary = (path.substr(extensionIndex + 1, extensionLength) == "glb");
		return loadGLTF_Implementation(meshes, meshTextureMap, directory, name, isBinary);
	}

	return false;
}

bool Scene::tryLoadFromFile(const std::string& path, VkDescriptorPool descPool)
{
	std::unordered_map<MeshIndex, SubmeshMaterials> meshTextureMap;
	if (!tryLoadSupportedFormat(path, m_meshes, meshTextureMap))
	{
		printf("Could not load scene file '%s', it did not match any of the supported formats.\n", path.c_str());
		return false;
	}

	/* ================= CREATE TEXTURES ================*/
	std::unordered_map<std::string, uint32_t> loadedTextures;
	StagingBufferPool stagingBufPool{};
	for (auto& kvPair : meshTextureMap)
	{
		for (auto& texPath : kvPair.second)
		{
			if (loadedTextures.count(texPath) == 0)
			{
				auto size = m_textures.size();
				m_textures.resize(size + 1);
				if (VkTexture2D::tryCreateTexture(m_textures.back(), texPath, m_presentationDevice, stagingBufPool, true))
				{
					loadedTextures[texPath] = size;
				} 
				else
				{
					m_textures.resize(size);
					printf("The texture at '%s' could not be loaded.\n", texPath.c_str());
				}
			}
		}
	}

	/* ================= CREATE MATERIALS ================*/
	auto* defaultShader = Shader::findShader(0);
	auto device = m_presentationDevice->getDevice();
	m_materials.resize(m_textures.size());
	for (int i = 0; i < m_textures.size(); i++)
	{
		m_presentationTarget->createMaterial(m_materials[i], device, descPool, defaultShader, m_textures[i].get());
	}

	/* ================= CREATE GRAPHICS MESHES ================*/
	auto defaultMeshDescriptor = Mesh::defaultMeshDescriptor;
	auto vmaAllocator = VkMemoryAllocator::getInstance()->m_allocator;
	const auto count = m_meshes.size();
	m_graphicsMeshes.resize(count);
	m_renderers.reserve(count);
	for (int i = 0; i < count; i++)
	{
		if (!m_meshes[i]->allocateGraphicsMesh(m_graphicsMeshes[i], vmaAllocator, m_presentationDevice, stagingBufPool) || !m_meshes[i]->isValid())
			return false;

		if (defaultMeshDescriptor != m_meshes[i]->getMeshDescriptor())
		{
			printf("Mesh metadata does not match - can not bind to the same pipeline.\n");
			return false;
		}

		auto& allMaterialIDs = meshTextureMap[i];
		uint32_t submeshIndex = 0;
		for (auto texPath : allMaterialIDs)
		{
			if (!loadedTextures.count(texPath) && loadedTextures[texPath] > 0 && loadedTextures[texPath] < m_materials.size())
			{
				printf("SceneLoader - The material not found for '%i' mesh.\n", i);
				continue;
			}

			m_renderers.push_back(MeshRenderer(m_graphicsMeshes[i].get(), submeshIndex, m_meshes[i]->getBounds(submeshIndex), &m_materials[loadedTextures[texPath]]->getMaterialVariant()));
			++submeshIndex;
		}
	}
	stagingBufPool.releaseAllResources();

	return true;
}