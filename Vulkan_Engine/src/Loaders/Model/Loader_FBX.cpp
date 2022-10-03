#include "pch.h"
#include "Loader_FBX.h"

#include "VkMesh.h"
#include "Mesh.h"
#include "Material.h"
#include "VkTypes/VkTexture.h"
#include "VkTypes/VkShader.h"

#include "FileManager/Path.h"
#include "FileManager/FileIO.h"

#include <OpenFBX/ofbx.h>

bool Loader::loadFBX_Implementation(std::vector<Mesh>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, std::vector<Transform>& transforms, const std::string& path, const std::string& name)
{
	auto fbxPath = Path(path + name + ".fbx");
	auto charCollection = FileIO::readFile(fbxPath);

	const ofbx::u8* ptr = reinterpret_cast<ofbx::u8*>(charCollection.data());
	const auto* scene = ofbx::load(ptr, static_cast<int>(charCollection.size()), static_cast<ofbx::u64>(ofbx::LoadFlags::IGNORE_BLEND_SHAPES));
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
		auto mIndexCount = static_cast<size_t>(geom->getIndexCount());
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
				Utility::assertIndex(mIndices[pin], mVertCount, meshName);
				Utility::assertIndex(mIndices[we], mVertCount, meshName);
				indices.push_back(mIndices[pin]);
				indices.push_back(mIndices[we]);

				if (mIndices[we + 1] >= 0)
				{
					Utility::assertIndex(mIndices[we + 1], mVertCount, meshName);
					indices.push_back(mIndices[we + 1]);

					we += 1;
				}
				else
				{
					Utility::assertIndex(~mIndices[we + 1], mVertCount, meshName);
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
		rendererIDs.push_back(Renderer(meshes.size() - 1, 0, { 0 }));
	}

	//for (auto& pair : polygonDistr)
	//{
	//	printf("Polygon edges {%i}, count {%i}.\n", pair.first, pair.second);
	//}

	transforms.push_back(Transform());
	materials.push_back(Material(0, TextureSource("C:/Git/Vulkan_Engine/Resources/Serialized/arch_stone_wall_01_Roughness.dds", VK_FORMAT_BC1_RGBA_SRGB_BLOCK, true)));

	return true;
}
