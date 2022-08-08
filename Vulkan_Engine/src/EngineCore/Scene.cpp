#include "pch.h"
#include "Scene.h"
#include "Color.h"

#include "vk_mem_alloc.h"
#include "tiny_obj_loader.h"

#include "VertexBinding.h"
#include "VkMesh.h"
#include "Mesh.h"
#include "Material.h"
#include "VkTypes/VkTexture.h"
#include "VkTypes/VkShader.h"

#include "Presentation/Device.h"
#include "PresentationTarget.h"

Mesh::Mesh(std::vector<glm::vec3>& positions, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors, std::vector<SubMesh>& submeshes)
	: m_positions(std::move(positions)), m_uvs(std::move(uvs)), m_normals(std::move(normals)), m_colors(std::move(colors)), m_submeshes(std::move(submeshes))
{
	updateMetaData();
}

Mesh::Mesh(std::vector<glm::vec3>& positions, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors, SubMesh& submesh)
	: m_positions(std::move(positions)), m_uvs(std::move(uvs)), m_normals(std::move(normals)), m_colors(std::move(colors)), m_submeshes(1)
{
	m_submeshes[0] = std::move(submesh);
	updateMetaData();
}

Mesh::Mesh(std::vector<glm::vec3>& positions, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors, SubMesh&& submesh)
	: m_positions(std::move(positions)), m_uvs(std::move(uvs)), m_normals(std::move(normals)), m_colors(std::move(colors)), m_submeshes(1) 
{
	m_submeshes[0] = std::move(submesh);
	updateMetaData();
}

void Mesh::clear()
{
	m_positions.clear();
	m_uvs.clear();
	m_normals.clear();
	m_colors.clear();

	m_submeshes.clear();
}

bool Mesh::validateOptionalBufferSize(size_t vectorSize, size_t vertexCount, char const* name)
{
	// Optional vector, but if it exists the size should match the position array.
	if (vectorSize > 0 && vectorSize != vertexCount)
	{
		printf("%s array size '%zu' does not match the position array size '%zu'.\n", name, vectorSize, vertexCount);
		return false;
	}

	return true;
}

bool Mesh::isValid()
{
	const auto n = m_positions.size();

	if (n == 0)
	{
		printf("The vertex array can not have 0 length.\n");
		return false;
	}

	// We won't support more than 65535 verts for now due to 16bit indexing
	if (n >= std::numeric_limits<uint16_t>::max())
	{
		printf("The vertex array size '%zu' exceeds the allowed capacity '%i'.\n", n, std::numeric_limits<uint16_t>::max());
		return false;
	}

	if (!validateOptionalBufferSize(m_uvs.size(), n, "Uvs") ||
		!validateOptionalBufferSize(m_normals.size(), n, "Normals") ||
		!validateOptionalBufferSize(m_colors.size(), n, "Colors"))
		return false;

#ifndef NDEBUG
	for (auto& submesh : m_submeshes)
	{
		for (size_t i = 0, size = submesh.m_indices.size(); i < size; ++i)
		{
			auto index = submesh.m_indices[i];
			if (index < 0 || index >= n)
			{
				printf("An incorrect index '%i' detected at position indices[%zu], should be in {0, %zu} range.\n", index, i, n);
				return false;
			}
		}
	}
#endif

	return true;
}

bool Scene::load(const VmaAllocator& vmaAllocator, VkDescriptorPool descPool)
{
	std::unordered_map<uint32_t, std::vector<uint32_t>> meshTextureMap;
	Scene::tryLoadFromFile(meshes, textures, meshTextureMap, "C:/Git/Vulkan_Engine/Resources/sponza.obj");

	auto* defaultShader = Shader::findShader(0);
	auto device = presentationDevice->getDevice();
	materials.resize(textures.size());
	for(int i = 0; i < textures.size(); i++)
	{
		presentationTarget->createMaterial(materials[i], device, descPool, defaultShader, textures[i].get());
	}

	auto defaultMeshDescriptor = Mesh::defaultMeshDescriptor;

	const auto count = meshes.size();
	graphicsMeshes.resize(count);
	renderers.reserve(count);
	for (int i = 0; i < count; i++)
	{
		if (!meshes[i]->allocateGraphicsMesh(graphicsMeshes[i], vmaAllocator) || !meshes[i]->isValid())
			return false;

		if (defaultMeshDescriptor != meshes[i]->getMeshDescriptor())
		{
			printf("Mesh metadata does not match - can not bind to the same pipeline.\n");
			return false;
		}

		auto& allMaterialIDs = meshTextureMap[i];
		uint32_t submeshIndex = 0;
		for (auto id : allMaterialIDs)
		{
			if (meshTextureMap.count(i) == 0 || id >= textures.size())
			{
				printf("SceneLoader - The material not found for '%i' mesh.\n", i);
				continue;
			}

			renderers.push_back(MeshRenderer(graphicsMeshes[i].get(), submeshIndex, &materials[id]->getMaterialVariant()));
			++submeshIndex;
		}
	}

	printf("Initialized the scene with (renderers = %zi), (meshes = %zi), (textures = %zi), (materials = %zi).\n", renderers.size(), meshes.size(), textures.size(), materials.size());
	return true;
}

void Scene::release(VkDevice device, const VmaAllocator& allocator)
{
	for (auto& mesh : meshes)
	{
		mesh->clear();
		mesh.release();
	}
	meshes.clear();

	for (auto& gmesh : graphicsMeshes)
	{
		gmesh->release(allocator);
		gmesh.release();
	}
	graphicsMeshes.clear();

	for (auto& tex : textures)
	{
		tex->release(device);
		tex.release();
	}
	textures.clear();

	for (auto& mat : materials)
	{
		mat->release(device);
		mat.release();
	}
	materials.clear();
}

Scene::Scene(const Presentation::Device* device, Presentation::PresentationTarget* target) 
	: presentationDevice(device), presentationTarget(target) { }
Scene::~Scene() {}

const std::vector<UNQ<Mesh>>& Scene::getMeshes() const { return meshes; }
const std::vector<UNQ<VkMesh>>& Scene::getGraphicsMeshes() const { return graphicsMeshes; }

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

bool Scene::loadObjImplementation(std::vector<UNQ<Mesh>>& meshes, std::vector<UNQ<VkTexture2D>>& textures, std::unordered_map<uint32_t, std::vector<uint32_t>>& meshTextureMap, const std::string& path, const std::string& name)
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

	tinyobj::LoadObj(&objAttribs, &objShapes, &objMats, &warn, &err, objPath.c_str(), path.c_str());

	textures.reserve(objMats.size());
	auto textureCount = 0;
	std::unordered_map<uint32_t, uint32_t> textureMap;
	for(int i = 0; i < objMats.size(); i++)
	{
		auto texPath = path + objMats[i].diffuse_texname;
		if (objMats[i].diffuse_texname.size() == 0 ||
			!std::filesystem::exists(texPath))
			continue;

		textureMap[i] = textureCount;
		textureCount += 1;

		textures.push_back(MAKEUNQ<VkTexture2D>(texPath, VkMemoryAllocator::getInstance()->m_allocator, presentationDevice));
	}

	//make sure to output the warnings to the console, in case there are issues with the file
	if (!warn.empty())
	{
		printf("warning: %s\n", warn.c_str());
	}

	//if we have any error, print it to the console, and break the mesh loading.
	//This happens if the file can't be found or is malformed
	if (!err.empty())
	{
		printf("Error: %s\n", err.c_str());
		return false;
	}

	struct SubMeshDesc
	{
		size_t indexCount;
		uint32_t mappedIndex;
	};

	std::unordered_map<uint32_t, SubMeshDesc> uniqueMaterialIDs;
	std::unordered_map<size_t, MeshDescriptor::TVertexIndices> indexMapping;
	//for (size_t i = 0; i < shapes.size(); i++)
	for (size_t i = 0; i < objShapes.size(); i++)
	{
		indexMapping.clear();

		auto& name = objShapes[i].name;
		auto& mesh = objShapes[i].mesh;
		auto& shapeIndices = mesh.indices;

		// The shape's vertex index here is referencing the big obj vertex buffer,
		// Our vertex buffer for mesh will be much smaller and should be indexed per-mesh, instead of globally.
		std::vector<SubMesh> submeshes;
		// std::vector<MeshDescriptor::TVertexIndices> indices(shapeIndices.size());

		if (mesh.material_ids.size() * 3 < shapeIndices.size())
		{
			printf("The material ID array size (3 * %zi) does not match the shapeIndices array size (%zi).", mesh.material_ids.size(), shapeIndices.size());
			return false;
		}

		uniqueMaterialIDs.clear();
		size_t materialCount = 0;
		for (size_t k = 0; k < mesh.material_ids.size(); k++)
		{
			auto id = mesh.material_ids[k];

			// if(id < 0 || id > objMats.size() || objMats[id].diffuse_texname.size() == 0 || !std::filesystem::exists(path + objMats[id].diffuse_texname))

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
			shapeMaterialCollection.push_back(textureMap[materialID]);
		}

		/*************************		VERTEX, NORMAL, UV indices -> pick unique and map to [0, N] range		**************************/
		struct KeyFuncs
		{
			static size_t cantor(size_t a, size_t b) { return (a + b + 1) * (a + b) / 2 + b; }
			static size_t cantor(int a, int b, int c) { return cantor(a, cantor(b, c)); }

			size_t operator()(const tinyobj::index_t& k)const
			{
				return cantor(k.vertex_index, k.normal_index, k.texcoord_index);
			}

			bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b)const
			{
				return a.vertex_index == b.vertex_index && a.normal_index == b.normal_index && a.texcoord_index == b.texcoord_index;
			}
		};

		MeshDescriptor::TVertexIndices mappedIndex = 0;
		std::unordered_map<tinyobj::index_t, MeshDescriptor::TVertexIndices, KeyFuncs, KeyFuncs> indmap;
		for (size_t k = 0; k < shapeIndices.size(); k++)
		{
			auto& indices = shapeIndices[k];
			auto curIndex = mappedIndex;

			if (indmap.count(indices) == 0)
			{
				indmap[indices] = mappedIndex;
				++mappedIndex;
			}
			else
			{
				curIndex = indmap[indices];
			}

			auto submeshIndex = uniqueMaterialIDs[mesh.material_ids[k / 3]].mappedIndex;
			submeshes[submeshIndex].m_indices.push_back(curIndex);
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

	// We are currently rendering the whole mesh (with all of its submeshes) with material[0] - hardcoded.
	// To do - integrate the submesh support. Can be problematic due to different vertex bindings (another pipeline?)
	printf("Loaded obj mesh at '%s' successfully, with %i meshes and %i diffuse textures.\n", 
		objPath.c_str(), as_uint32(meshes.size()), as_uint32(textures.size()));
	return true;
}

bool Scene::tryLoadFromFile(std::vector<UNQ<Mesh>>& meshes, std::vector<UNQ<VkTexture2D>>& textures, std::unordered_map<uint32_t, std::vector<uint32_t>>& meshTextureMap, const std::string& path)
{
	const std::string supportedFormat = ".obj";
	if (path.length() > supportedFormat.length() && std::equal(supportedFormat.rbegin(), supportedFormat.rend(), path.rbegin()))
	{
		auto nameStart = path.find_last_of('/') + 1;
		auto extensionLength = supportedFormat.length();
		auto directory = path.substr(0, nameStart);
		auto name = path.substr(nameStart, path.length() - nameStart - extensionLength);

		loadObjImplementation(meshes, textures, meshTextureMap, directory, name);
	}

	return false;
}