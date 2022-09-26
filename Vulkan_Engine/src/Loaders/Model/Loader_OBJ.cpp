#include "pch.h"
#include "Loader_OBJ.h"
#include "Loaders/Model/Common.h"

#include "VkMesh.h"
#include "Mesh.h"
#include "Material.h"
#include "VkTypes/VkTexture.h"
#include "VkTypes/VkShader.h"

#include "Profiling/ProfileMarker.h"

bool Loader::loadOBJ_Implementation(std::vector<Mesh>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, std::vector<Transform>& transforms, const std::string& path, const std::string& name)
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
			auto vert = Utility::reinterpretAt<float, MeshDescriptor::TVertexPosition>(objAttribs.vertices, indices.vertex_index);
			Utility::minVector(boundsMinMax[submeshIndex * 2], vert);
			Utility::maxVector(boundsMinMax[submeshIndex * 2 + 1], vert);

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

			vertices[kv.second] = Utility::reinterpretAt<float, MeshDescriptor::TVertexPosition>(objAttribs.vertices, vi);

			normals[kv.second] = Utility::reinterpretAt_orFallback<float, MeshDescriptor::TVertexNormal>(objAttribs.normals, ni);
			uvs[kv.second] = Utility::reinterpretAt_orFallback<float, MeshDescriptor::TVertexUV>(objAttribs.texcoords, uvi);
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
