#pragma once
#include "pch.h"
#include "Path.h"

#define EDITOR

class Directories
{
public:
	static Path getApplicationPath();
	static Path getAbsolutePath(const std::string& path);
	
	static Path getWorkingDirectory();
	static Path getWorkingScene();
	static Path getWorkingModel();
	static Path getShaderLibraryPath();

private:
	inline static std::string library_relative = "Resources/Library/";
	inline static std::string libraryShaderPath_relative = "Resources/Library/outputSPV/";

	inline static std::string workingSceneName = "Scene.binary";
	inline static std::string workingSceneDir_relative = "Resources/Serialized/";
	inline static std::string workingModel_relative = "Resources/deccer_cubes.fbx";

	inline static std::string CRYTEK_SPONZA_OBJ = "C:/Git/Vulkan_Engine/Resources/sponza.obj";
	inline static std::string CRYTEK_SPONZA_FBX = "C:/Git/Vulkan_Engine/Resources/sponza.fbx";
	inline static std::string LIGHTING_SCENE = "C:/Git/Vulkan_Engine/Resources/debrovic_sponza/sponza.obj";
	inline static std::string CUBE_GLTF = "C:/Git/Vulkan_Engine/Resources/gltf/Cube_khr.gltf";
};