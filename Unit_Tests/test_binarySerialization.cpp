#include <pch.h>
#include "gtest/gtest.h"
#include "Scene.h"
#include "Material.h"
#include "Mesh.h"

#include <iostream>
#include <sstream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>

const static inline std::string k_scenePath = "C:/Git/Vulkan_Engine/Resources/sponza.obj";
const static inline std::string k_testDirectory = "C:/Git/test_dir/";

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

TEST(Serialization, SceneBinary)
{
	Scene scene(nullptr, nullptr);
	scene.tryLoadSupportedFormat(k_scenePath);

	EXPECT_TRUE(scene.getMeshes().size() > 0);

	const auto fileName = "file.binary";
	const auto fullPath = k_testDirectory + fileName;

	// WRITE
	{
		auto stream = std::fstream(fullPath, std::ios::out | std::ios::binary);
		boost::archive::binary_oarchive archive(stream);
		EXPECT_TRUE(stream.is_open());

		archive << scene;

		stream.close();
	}

	// READ
	{
		auto stream = std::fstream(fullPath, std::ios::in | std::ios::binary);
		boost::archive::binary_iarchive archive(stream);

		Scene loadedScene(nullptr, nullptr);
		archive >> loadedScene;

		EXPECT_TRUE(stream.is_open());

		// Meshes
		const auto& meshes = scene.getMeshes();
		const auto& loadedMeshes = loadedScene.getMeshes();
		{
			EXPECT_TRUE(loadedMeshes.size() == meshes.size());

			for (size_t i = 0; i < loadedMeshes.size(); i++)
			{
				EXPECT_EQ(loadedMeshes[i], meshes[i]);
			}
		}

		// Materials
		const auto& materials = scene.getMaterials();
		const auto& loadedMaterials = loadedScene.getMaterials();
		{
			EXPECT_TRUE(loadedMaterials.size() == materials.size());

			for (size_t i = 0; i < loadedMaterials.size(); i++)
			{
				EXPECT_EQ(loadedMaterials[i], materials[i]);
			}
		}

		// Renderers
		const auto& renderers = scene.getRendererIDs();
		const auto& loadedRenderers = loadedScene.getRendererIDs();
		{
			EXPECT_TRUE(loadedRenderers.size() == renderers.size());

			for (size_t i = 0; i < loadedRenderers.size(); i++)
			{
				EXPECT_EQ(loadedRenderers[i], renderers[i]);
			}
		}
		stream.close();
	}
}
