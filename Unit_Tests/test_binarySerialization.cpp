#include <pch.h>
#include "gtest/gtest.h"
#include "Scene.h"
#include "Material.h"
#include "Mesh.h"
#include "FileManager/Directories.h"

#include <iostream>
#include <sstream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>

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

TEST(Texturesource, Path)
{
	auto test1 = TextureSource("directory/file.png");
	auto test2 = TextureSource("long_complex/white space\\directory\\file.png");
	auto test3 = TextureSource("file.png");

	EXPECT_EQ(test1.getTextureName(false), "file");
	EXPECT_EQ(test1.getTextureName(true), "file.png");

	EXPECT_EQ(test2.getTextureName(false), "file");
	EXPECT_EQ(test2.getTextureName(true), "file.png");

	EXPECT_EQ(test3.getTextureName(false), "file");
	EXPECT_EQ(test3.getTextureName(true), "file.png");

	test1.path.removeDirectory("directory");
	EXPECT_EQ(test1.path.value, "file.png");

	test2.path.removeDirectory("long_complex/white space/directory/");
	EXPECT_EQ(test2.path.value, "file.png");

	auto test4 = Path("C:\\Git\\Vulkan_Engine\\Resources\\Serialized/background.dds");
	test4.removeDirectory( Directories::getWorkingDirectory() );
	EXPECT_EQ(test4.value, "background.dds");
	EXPECT_EQ(Directories::getWorkingDirectory().combine(test4.value).value, "C:/Git/Vulkan_Engine/Resources/Serialized/background.dds");
}

TEST(Serialization, SceneBinary)
{
	Scene scene(nullptr, nullptr);
	const auto modelPath = Directories::getWorkingModel();
	const auto fullPath = Directories::getWorkingScene();
	
	scene.tryLoadSupportedFormat(modelPath);
	EXPECT_TRUE(scene.getMeshes().size() > 0);

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
