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

float genFloat(float LO, float HI)
{
	return LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
}

TEST(Benchmark, Serialization)
{
	std::string path1 = "C:/Git/test_dir/bigFlatArray.bin",
		path2 = "C:/Git/test_dir/bigVector.bin";

	std::vector<float> bigFlatArray;
	std::vector<glm::vec4> bigVector;

	const int COUNT = 10000000;
	const float RANGE = 10000.f;
	bigFlatArray.resize(COUNT);
	bigVector.resize(COUNT / 4);

	const float MIN = - RANGE / 2;
	const float MAX = RANGE / 2;
	for (int i = 0; i < COUNT / 4; i++)
	{
		float a = genFloat(MIN, MAX),
			b = genFloat(MIN, MAX),
			c = genFloat(MIN, MAX),
			d = genFloat(MIN, MAX);

		bigFlatArray[i * 4 + 0] = a;
		bigFlatArray[i * 4 + 1] = b;
		bigFlatArray[i * 4 + 2] = c;
		bigFlatArray[i * 4 + 3] = d;

		bigVector[i] = glm::vec4(a, b, c, d);
	}

	{
		{
			ProfileMarker _("WRITE - Big Flat Array");
			auto stream = std::fstream(path1, std::ios::out | std::ios::binary);
			boost::archive::binary_oarchive archive(stream);

			archive << bigFlatArray;
			stream.close();
		}
		bigFlatArray.clear();
		bigFlatArray = std::vector<float>();

		{
			ProfileMarker _("READ - Big Flat Array");
			auto stream = std::fstream(path1, std::ios::in | std::ios::binary);
			boost::archive::binary_iarchive archive(stream);

			archive >> bigFlatArray;
			stream.close();
		}
	}

	{
		{
			ProfileMarker _("WRITE - Big Vector 4");
			auto stream = std::fstream(path2, std::ios::out | std::ios::binary);
			boost::archive::binary_oarchive archive(stream);

			archive << bigVector;
			stream.close();
		}
		bigVector.clear();
		bigVector = std::vector<glm::vec4>();

		{
			ProfileMarker _("READ - Big Vector 4");
			auto stream = std::fstream(path2, std::ios::in | std::ios::binary);
			boost::archive::binary_iarchive archive(stream);

			archive >> bigVector;
			stream.close();
		}
	}
}


/*
TEST(Texturesource, Path)
{
	return;

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
	const auto modelPaths = Directories::getWorkingModels();
	const auto fullPath = Directories::getWorkingScene();
	
	for (auto& model : modelPaths)
	{
		auto meshCount = scene.getMeshes().size();
		scene.tryLoadSupportedFormat(model);

		EXPECT_TRUE(scene.getMeshes().size() - meshCount > 0);
	}

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

		// Transforms
		const auto& transforms = scene.getTransforms();
		const auto& loadedTransforms = loadedScene.getTransforms();
		{
			EXPECT_TRUE(transforms.size() == loadedTransforms.size());

			for (size_t i = 0; i < loadedTransforms.size(); i++)
			{
				EXPECT_EQ(loadedTransforms[i], transforms[i]);
			}
		}

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
*/