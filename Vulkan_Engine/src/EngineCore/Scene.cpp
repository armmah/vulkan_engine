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

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "OpenFBX/ofbx.h"

Scene::Scene(const Presentation::Device* device, Presentation::PresentationTarget* target)
	: m_presentationDevice(device), m_presentationTarget(target) { }

Scene::~Scene() {}

bool Scene::load(VkDescriptorPool descPool)
{
	ProfileMarker _("Scene::load");

	auto path = Directories::getWorkingScene();
	//auto path = Directories::getWorkingModels()[1];
	tryLoadFromFile(path, descPool);

	printf("Initialized the scene with (renderers = %zi), (transforms = %zi), (meshes = %zi), (textures = %zi), (materials = %zi).\n", m_renderers.size(), m_transforms.size(), m_meshes.size(), m_textures.size(), m_materials.size());
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
const std::vector<Transform>& Scene::getTransforms() const { return m_transforms; }
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

void assertIndex(int index, int vertCount, const char* name)
{
	if (index >= std::numeric_limits<MeshDescriptor::TVertexIndices>::max())
		printf("The index of mesh '%s' exceeded the 16 bit precision limit with id {%i}.\n", name, index);

	if (index < 0 || index >= vertCount)
		printf("The index of mesh '%s' was pointing outside of the vertex array {0 <= %i < %i}.\n", name, index, vertCount);
}

glm::mat4 convertToGLM(aiMatrix4x4 src)
{
	// row-major to column-major
	// [x, y] => [y, x]
	return glm::mat4(
		src.a1, src.b1, src.c1, src.d1,
		src.a2, src.b2, src.c2, src.d2,
		src.a3, src.b3, src.c3, src.d3,
		src.a4, src.b4, src.c4, src.d4
	);
}

void crawl(std::vector<Transform>& globalTransformCollection, std::unordered_map<int, size_t>& meshToTransform, aiNode* node, aiMatrix4x4 parentMatrix, int depth)
{
	auto localMatrix = parentMatrix * node->mTransformation;

	auto mat = convertToGLM(localMatrix);
	globalTransformCollection.push_back(Transform( convertToGLM(localMatrix) ));

	for (unsigned int mi = 0; mi < node->mNumMeshes; mi++)
	{
		auto meshID = node->mMeshes[mi];
		meshToTransform[meshID] = globalTransformCollection.size() - 1;
	}

#ifdef VERBOSE_INFO_MESSAGE
	std::string padding = "";
	for (int i = 0; i < depth; i++)
		padding += '\t';

	printf("%s%s\n", padding.c_str(), node->mName.C_Str());
#endif
	auto** childPtr = node->mChildren;
	for(unsigned int chi = 0; chi < node->mNumChildren; chi++)
	{
		if(childPtr[chi]) 
			crawl(globalTransformCollection, meshToTransform, childPtr[chi], localMatrix, depth + 1);
	}
}

bool load_AssimpImplementation(std::vector<Mesh>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, std::vector<Transform>& transforms, const Path& fullPath)
{
	// Create an instance of the Importer class
	Assimp::Importer importer;

	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	const aiScene* scene = importer.ReadFile(fullPath.c_str(),
		aiProcess_Triangulate |
		aiProcess_SortByPType);

	// If the import failed, report it
	if (!scene) 
	{
		printf("Loader Error: '%s'.\n", importer.GetErrorString());
		return false;
	}

	auto existingElementsCount = meshes.size();
	meshes.reserve(existingElementsCount + scene->mNumMeshes);
	rendererIDs.reserve(existingElementsCount + scene->mNumMeshes);
	materials.reserve(materials.size() + scene->mNumMaterials);
	transforms.reserve(transforms.size() + scene->mNumMeshes);

	// Now we can access the file's contents.
	std::unordered_map<int, size_t> meshToTransform;
	{
		ProfileMarker _("	Loader::Crawl_Nodes");
		crawl(transforms, meshToTransform, scene->mRootNode, aiMatrix4x4(), 0);
	}

	std::unordered_map<int, std::vector<int>> textureToMeshMap;

	ProfileMarker _("	Loader::CreateMesh");
	std::vector<MeshDescriptor::TVertexIndices> indices;
	std::vector<MeshDescriptor::TVertexPosition> vertices;
	std::vector<MeshDescriptor::TVertexNormal> normals;
	std::vector<MeshDescriptor::TVertexUV> uvs;
	std::vector<MeshDescriptor::TVertexColor> colors;
	for (unsigned int mi = 0; mi < scene->mNumMeshes; mi++)
	{
		const auto* mesh = scene->mMeshes[mi];
		auto vertN = mesh->mNumVertices;

		//std::string str = mesh->mName.C_Str();
		//if (str.find("columns_1stfloor") == std::string::npos)
		//	continue;

		vertices.resize(vertN);
		normals.resize(vertN);
		uvs.resize(vertN);
		//colors.resize(vertN);

		const auto* mVerts = mesh->mVertices;
		const auto* mTexcoords = mesh->mTextureCoords[0];
		const auto* mNorms = mesh->mNormals;
#ifdef ASSIMP_DOUBLE_PRECISION
		for (int vi = 0; vi < vertN; vi++)
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
#else
		reinterpretCopy(mVerts, vertN, vertices);
		reinterpretCopy(mNorms, vertN, normals);

		for (int vi = 0; vi < vertN; vi++)
		{
			uvs[vi] = glm::vec2(mTexcoords[vi].x, mTexcoords[vi].y);
		}
#endif

		for (unsigned int fi = 0; fi < mesh->mNumFaces; fi++)
		{
			auto& face = mesh->mFaces[fi];
			assert(face.mNumIndices == 3);

			for(unsigned int ii = 0; ii < face.mNumIndices; ii++)
			{
				assertIndex(face.mIndices[ii], vertN, mesh->mName.C_Str());

				indices.push_back( static_cast<unsigned short>(face.mIndices[ii]) );
			}

		}
		auto matIndex = mesh->mMaterialIndex;

		std::vector<SubMesh> submeshes(1);
		submeshes[0] = SubMesh(indices);

		auto boundsMin = mesh->mAABB.mMin,
			boundsMax = mesh->mAABB.mMax;
		//submeshes[0].m_bounds = BoundsAABB(
		//	glm::vec3(boundsMin.x, boundsMin.y, boundsMin.z), 
		//	glm::vec3(boundsMax.x, boundsMax.y, boundsMax.z)
		//);

		meshes.push_back(Mesh(vertices, uvs, normals, colors, submeshes));

		// By default set the material ID to 0, when its loaded and processed successfuly will be replaced by actual ID.
		auto meshFinalIndex = meshes.size() - 1;
		rendererIDs.push_back(Renderer(meshFinalIndex, meshToTransform[mi], {0}));
		textureToMeshMap[matIndex].push_back(meshFinalIndex);
	}

	auto dir = fullPath.getFileDirectory();
	for (unsigned int mi = 0; mi < scene->mNumMaterials; mi++)
	{
		const auto* mat = scene->mMaterials[mi];

		aiString res;
		if (mat->Get("$tex.file", aiTextureType::aiTextureType_DIFFUSE, 0, res) == AI_SUCCESS)
		{
			auto texPath = dir + res.C_Str();
			if (fileExists(texPath))
			{
				materials.push_back(Material(0, TextureSource(std::move(texPath), VK_FORMAT_R8G8B8A8_SRGB, true)));

				// Replace the material ID, if successfuly loaded the texture, for all meshes referencing it.
				for (auto& meshID : textureToMeshMap[mi])
				{
					if (meshID >= rendererIDs.size())
						continue;
					rendererIDs[meshID].materialIDs[0] = materials.size() - 1;
				}
			}
			else printf("Loader: Could not find texture at path '%s'.\n", texPath.c_str());
		}
		else printf("Loader: The diffuse texture did not exist in material '%s' properties.\n", mat->GetName().C_Str());
	}

	// Fallback - if all textures are missing.
	if (meshes.size() > 0 && materials.size() == 0)
	{
		// To do - replace with a default 1x1 white texture
		materials.push_back(Material(0, TextureSource("C:/Git/Vulkan_Engine/Resources/Serialized/arch_stone_wall_01_Roughness.dds", VK_FORMAT_BC1_RGBA_SRGB_BLOCK, true)));
	}

	return true;
}

bool loadFBX_Implementation(std::vector<Mesh>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, std::vector<Transform>& transforms, const std::string& path, const std::string& name)
{
	auto fbxPath = Path(path + name + ".fbx");
	auto charCollection = FileIO::readFile(fbxPath);

	const ofbx::u8* ptr = reinterpret_cast<ofbx::u8*>(charCollection.data());
	const auto* scene = ofbx::load(ptr, charCollection.size(), static_cast<ofbx::u64>(ofbx::LoadFlags::IGNORE_BLEND_SHAPES));
	if (!scene) return false;

	std::unordered_map<int, int> polygonDistr;
	for (size_t meshID = 0; meshID < scene->getMeshCount(); meshID++)
	{
		auto* mesh = scene->getMesh(static_cast<int>(meshID));
		auto meshName = mesh->name;
		auto* geom = mesh->getGeometry();
		
		auto* mVerts = geom->getVertices();
		auto mVertCount = geom->getVertexCount();

		auto* mTexcoords = geom->getUVs(0);
		auto* mNorms = geom->getNormals();

		std::vector<MeshDescriptor::TVertexPosition> vertices(mVertCount);
		std::vector<MeshDescriptor::TVertexNormal> normals(mVertCount);
		std::vector<MeshDescriptor::TVertexUV> uvs(mVertCount);
		std::vector<MeshDescriptor::TVertexColor> colors;

		for (int vi = 0; vi < mVertCount; vi++)
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

		/*
		auto polygonCount = 0;
		for (int ii = 0; ii < mIndexCount; ii++)
		{
			polygonCount += 1;

			if (mIndices[ii] < 0)
			{
				polygonDistr[polygonCount] += 1;
				polygonCount = 0;
			}
		}*/

		for (int ii = 0; ii < mIndexCount;)
		{
			auto pin = ii;
			auto we = ii + 1;

			while (we + 1 < mIndexCount)
			{
				assertIndex(mIndices[pin], mVertCount, meshName);
				assertIndex(mIndices[we], mVertCount, meshName);
				indices.push_back(mIndices[pin]);
				indices.push_back(mIndices[we]);

				if (mIndices[we + 1] >= 0)
				{
					assertIndex(mIndices[we + 1], mVertCount, meshName);
					indices.push_back(mIndices[we + 1]);

					we += 1;
				}
				else
				{
					assertIndex(~mIndices[we + 1], mVertCount, meshName);
					indices.push_back(~mIndices[we + 1]);

					ii = we + 2;
					break;
				}

			}
		}

		/*
		int bufIndex = 0;
		for (int ii = 0; ii + 2 < mIndexCount; ii += 3)
		{
			auto a = mIndices[ii + 0],
				b = mIndices[ii + 1],
				c = mIndices[ii + 2];

			if (b < 0)
			{
				printf("Not handled - second index \n");
				ii -= 1;

				continue;
			}

			if (c < 0)
			{
				c = ~c;

				assertIndex(a, mVertCount, meshName);
				assertIndex(b, mVertCount, meshName);
				assertIndex(c, mVertCount, meshName);

				indices.push_back(a);
				indices.push_back(b);
				indices.push_back(c);

				continue;
			}

			if (mIndices[ii + 3] < 0)
			{
				auto d = ~mIndices[ii + 3];
				ii += 1;

				assertIndex(a, mVertCount, meshName);
				assertIndex(b, mVertCount, meshName);
				assertIndex(c, mVertCount, meshName);
				assertIndex(d, mVertCount, meshName);

				indices.push_back(a);
				indices.push_back(b);
				indices.push_back(d);
				indices.push_back(d);
				indices.push_back(b);
				indices.push_back(c);

				continue;
			}

			ii += 1;
			printf("[wtf] Not handled - all four indices were positive > 0 ? \n");
		}

		/*
		
			mIndices[ii + 0]
			mIndices[ii + 1]
			mIndices[ii + 3]
			mIndices[ii + 3]
			mIndices[ii + 1]
			mIndices[ii + 2]
		
		*/


		std::vector<SubMesh> submeshes(1);
		submeshes[0] = SubMesh(indices);

		meshes.push_back(Mesh(vertices, uvs, normals, colors, submeshes));
		rendererIDs.push_back(Renderer(meshes.size() - 1, 0, {0}));
	}

	//for (auto& pair : polygonDistr)
	//{
	//	printf("Polygon edges {%i}, count {%i}.\n", pair.first, pair.second);
	//}

	transforms.push_back(Transform());
	materials.push_back(Material(0, TextureSource("C:/Git/Vulkan_Engine/Resources/Serialized/arch_stone_wall_01_Roughness.dds", VK_FORMAT_BC1_RGBA_SRGB_BLOCK, true)));

	return true;
}

bool Scene::loadOBJ_Implementation(std::vector<Mesh>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, std::vector<Transform>& transforms, const std::string& path, const std::string& name)
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
	meshes.reserve(meshes.size() + objShapes.size());
	rendererIDs.reserve(meshes.size() + objShapes.size());
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
	materials.reserve(materials.size() + objMats.size());
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

		rendererIDs.push_back(Renderer(kv.first, 0, globalBufMaterialIDs));
	}

	if (transforms.size() == 0)
	{
		transforms.push_back(Transform());
	}

	return true;
}

namespace boost::serialization
{
	template <typename Ar>
	void serialize(Ar& ar, glm::vec2& v, unsigned _)
	{
		ar& v.x & v.y;
	}

	template <typename Ar>
	void serialize(Ar& ar, glm::vec3& v, unsigned _)
	{
		ar& v.x & v.y & v.z;
	}

	template <typename Ar>
	void serialize(Ar& ar, glm::vec4& v, unsigned _)
	{
		ar& v.x & v.y & v.z & v.w;
	}

	template <typename Ar>
	void serialize(Ar& ar, glm::mat4& m, unsigned _)
	{
		ar& m[0];
		ar& m[1];
		ar& m[2];
		ar& m[3];
	}
}

bool Scene::tryLoadSupportedFormat(const Path& path)
{
	auto directory = path.getFileDirectory();
	auto name = path.getFileName(false);

	/* ================ READ SERIALIZED BINARY =============== */
	if (fileExists(path, ".binary"))
	{
		ProfileMarker _("Loader::Binary");

		// Deserialize
		auto stream = std::fstream(path, std::ios::in | std::ios::binary);
		boost::archive::binary_iarchive archive(stream);

		archive >> *this;

		return true;
	}

	/* ================ READ FROM GLTF =============== */
	if (fileExists(path, ".gltf"))
	{
		ProfileMarker _("Loader::ASSIMP");
		return load_AssimpImplementation(m_meshes, m_materials, m_rendererIDs, m_transforms, Path(path));
	}

	/* ================ READ FROM OBJ =============== */
	if (fileExists(path, ".obj"))
	{
		ProfileMarker _("Loader::Custom_OBJ");
		return loadOBJ_Implementation(m_meshes, m_materials, m_rendererIDs, m_transforms, directory, name);
	}

	/* ================ READ FROM FBX =============== */
	if (fileExists(path, ".fbx"))
	{
		ProfileMarker _("Loader::Custom_FBX");
		return loadFBX_Implementation(m_meshes, m_materials, m_rendererIDs, m_transforms, directory, name);
	}

	// Fallback
	if (fileExists(path))
	{
		ProfileMarker _("Loader::ASSIMP");
		return load_AssimpImplementation(m_meshes, m_materials, m_rendererIDs, m_transforms, Path(path));
	}

	return false;
}

bool Scene::tryLoadFromFile(const Path& path, VkDescriptorPool descPool)
{
	if (!tryLoadSupportedFormat(path))
	{
		printf("Could not load scene file '%s', it did not match any of the supported formats.\n", path.c_str());
		return false;
	}

	std::unordered_map<TextureSource, uint32_t> loadedTextures;
	StagingBufferPool stagingBufPool{};
	{
		ProfileMarker _("Scene::Create_Graphics_Materials");
		/* ================= CREATE TEXTURES ================*/
		/* ================= CREATE GRAPHICS MATERIALS ================*/
		m_graphicsMaterials.reserve(m_textures.size());
		auto device = m_presentationDevice->getDevice();
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
						rendererIDs.materialIDs = { 0 };

						m_textures.resize(size);
						m_graphicsMaterials.resize(size);
						printf("The texture at '%s' could not be loaded.\n", texSrc.path.c_str());
					}
				}
			}
		}
	}

	{
		ProfileMarker _("Scene::Create_Graphics_Meshes");
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
				continue;

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

				m_renderers.push_back(
					VkMeshRenderer(
						&m_graphicsMeshes.back(), submeshIndex,
						&m_graphicsMaterials[loadedTextures[texPath]]->getMaterialVariant(),
						mesh.getBounds(submeshIndex), &m_transforms[ids.transformID]
					)
				);
				++submeshIndex;
			}
		}
	}
	stagingBufPool.releaseAllResources();

	return true;
}
