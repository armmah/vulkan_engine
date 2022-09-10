#pragma once
#include "pch.h"
#include "Path.h"

#define EDITOR

class Directories
{
public:
	static Path getApplicationPath()
	{
		std::string pathString;
#ifndef EDITOR
		char buffer[MAX_PATH] = {};
		::GetSystemDirectoryA(buffer, _countof(buffer));
		strcat(buffer, "\\version.dll");
		pathString = std::string(buffer);
#else
		pathString = "C:/Git/Vulkan_Engine/";
#endif

		return Path(std::move(pathString));
	}

	static Path getAbsolutePath(const std::string& path) { return getApplicationPath().combine(path); }
	
	static Path getWorkingDirectory() { return getAbsolutePath( workingSceneDir_relative ); }
	static Path getWorkingScene() { return getAbsolutePath( workingSceneDir_relative ).combine(workingSceneName); }
	static Path getWorkingModel() { return getAbsolutePath( workingModel_relative ); }
	
	static Path getShaderLibraryPath() { return getAbsolutePath( libraryShaderPath_relative ); }

private:
	inline static std::string library_relative = "Resources/Library/";
	inline static std::string libraryShaderPath_relative = "Resources/Library/outputSPV/";

	inline static std::string workingSceneName = "Scene.binary";
	inline static std::string workingSceneDir_relative = "Resources/Serialized/";
	inline static std::string workingModel_relative = "Resources/sponza.obj";

	inline static std::string CRYTEK_SPONZA = "C:/Git/Vulkan_Engine/Resources/sponza.obj";
	inline static std::string LIGHTING_SCENE = "C:/Git/Vulkan_Engine/Resources/debrovic_sponza/sponza.obj";
	inline static std::string CUBE_GLTF = "C:/Git/Vulkan_Engine/Resources/gltf/Cube_khr.gltf";
};