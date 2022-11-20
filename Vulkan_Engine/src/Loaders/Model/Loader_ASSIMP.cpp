#include "pch.h"
#include "Loader_ASSIMP.h"
#include "Loaders/Model/Common.h"
#include "Loaders/Model/ModelLoaderOptions.h"

#include "VkMesh.h"
#include "Mesh.h"
#include "Material.h"
#include "VkTypes/VkTexture.h"
#include "VkTypes/VkShader.h"

#include "Profiling/ProfileMarker.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

glm::mat4 Loader::convertToGLM(const aiMatrix4x4& src)
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

void Loader::crawl(std::vector<Transform>& globalTransformCollection, std::unordered_map<int, size_t>& meshToTransform, aiNode* node, const aiMatrix4x4& parentMatrix, int depth)
{
	auto localMatrix = parentMatrix * node->mTransformation;

	globalTransformCollection.push_back(Transform(Loader::convertToGLM(localMatrix)));

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
	for (unsigned int chi = 0; chi < node->mNumChildren; chi++)
	{
		if (childPtr[chi])
			Loader::crawl(globalTransformCollection, meshToTransform, childPtr[chi], localMatrix, depth + 1);
	}
}

bool getTexPath(std::string& fullPath, const std::string& modelDirectory, const aiMaterial* mat, aiTextureType texType)
{
	aiString res;
	if (mat->Get(_AI_MATKEY_TEXTURE_BASE, texType, 0, res) == AI_SUCCESS)
	{
		fullPath = modelDirectory + res.C_Str();

		if (fileExists(fullPath))
		{
			return true;
		}
		else
		{
			printf("The texture (%i, '%s') was not found at path '%s'.\n", texType, res.C_Str(), fullPath.c_str());
			return false;
		}
	}

	return false;
}

bool Loader::load_AssimpImplementation(std::vector<Mesh>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, std::vector<Transform>& transforms, 
	const Loader::ModelLoaderOptions& options)
{
	const auto& fullPath = options.filePath;
	const auto modelScaler = options.sizeModifier;

	// Create an instance of the Importer class
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(fullPath.c_str(),
		aiProcess_Triangulate |
		aiProcess_GenBoundingBoxes |
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

	std::unordered_map<int, size_t> meshToTransform;
	{
		ProfileMarker _("	Loader::Crawl_Nodes");
		Loader::crawl(transforms, meshToTransform, scene->mRootNode, aiMatrix4x4(), 0);
	}

	std::unordered_map<int, std::vector<size_t>> textureToMeshMap;

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

#ifdef IMPORT_LIMITER_ENABLED
		if (import_mesh_limitter > 0 && meshes.size() > import_mesh_limitter)
			break;
#endif

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
			) * modelScaler;

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
		Utility::reinterpretCopy(mVerts, vertN, vertices);
		Utility::reinterpretCopy(mNorms, vertN, normals);

		if (modelScaler != 1.0)
		{
			for (int vi = 0; vi < vertN; vi++)
			{
				vertices[vi] *= modelScaler;
			}
		}

		for (unsigned int vi = 0; vi < vertN; vi++)
		{
			uvs[vi] = glm::vec2(mTexcoords[vi].x, mTexcoords[vi].y);
		}
#endif

		for (unsigned int fi = 0; fi < mesh->mNumFaces; fi++)
		{
			auto& face = mesh->mFaces[fi];
			assert(face.mNumIndices == 3);

			for (unsigned int ii = 0; ii < face.mNumIndices; ii++)
			{
				Utility::assertIndex(face.mIndices[ii], vertN, mesh->mName.C_Str());

				indices.push_back(static_cast<MeshDescriptor::TVertexIndices>(face.mIndices[ii]));
			}
		}
		auto matIndex = mesh->mMaterialIndex;

		std::vector<SubMesh> submeshes(1);
		submeshes[0] = SubMesh(indices);

		auto boundsMin = mesh->mAABB.mMin * modelScaler,
			boundsMax = mesh->mAABB.mMax * modelScaler;
		submeshes[0].m_bounds = BoundsAABB(
			glm::vec3(boundsMin.x, boundsMin.y, boundsMin.z),
			glm::vec3(boundsMax.x, boundsMax.y, boundsMax.z)
		);

		meshes.push_back(Mesh(vertices, uvs, normals, colors, submeshes));

		// By default set the material ID to 0, when its loaded and processed successfuly will be replaced by actual ID.
		auto meshFinalIndex = meshes.size() - 1;
		rendererIDs.push_back(Renderer(meshFinalIndex, meshToTransform[mi], { 0 }));
		textureToMeshMap[matIndex].push_back(meshFinalIndex);
	}

	auto dir = fullPath.getFileDirectory();
	for (unsigned int mi = 0; mi < scene->mNumMaterials; mi++)
	{
		const auto* mat = scene->mMaterials[mi];

		std::string diffusePath;
		if (getTexPath(diffusePath, dir, mat, aiTextureType::aiTextureType_DIFFUSE))
		{
			materials.push_back(Material(0, TextureSource(std::move(diffusePath), VK_FORMAT_R8G8B8A8_SRGB, true)));

			// Replace the material ID, if successfuly loaded the texture, for all meshes referencing it.
			for (auto& meshID : textureToMeshMap[mi])
			{
				if (meshID >= rendererIDs.size())
					continue;
				rendererIDs[meshID].materialIDs[0] = materials.size() - 1;
			}
		}
		else printf("Loader: The diffuse texture did not exist in material '%s' properties.\n", mat->GetName().C_Str());

		std::string normalPath;
		if (getTexPath(normalPath, dir, mat, aiTextureType::aiTextureType_NORMALS))
		{
		}

		std::string metalness;
		if (getTexPath(metalness, dir, mat, aiTextureType::aiTextureType_METALNESS))
		{
		}

		std::string roughness;
		if(getTexPath(roughness, dir, mat, aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS))
		{
		}

		/*
		if (metalness == roughness)
		{
			printf("Normal = '%s', Data = '%s'.\n", normalPath.c_str(), roughness.c_str());
		}
		else
		{
			printf("Normal = '%s', Metal = '%s', Roughness = '%s'.\n", normalPath.c_str(), metalness.c_str(), roughness.c_str());
		}*/
	}

	return true;
}
