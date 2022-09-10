#include "pch.h"
#include "Directories.h"

Path Directories::getAbsolutePath(const std::string& path) { return getApplicationPath().combine(path); }

Path Directories::getApplicationPath()
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

Path Directories::getWorkingDirectory() { return getAbsolutePath(workingSceneDir_relative); }

Path Directories::getWorkingScene() { return getAbsolutePath(workingSceneDir_relative).combine(workingSceneName); }

Path Directories::getWorkingModel() { return getAbsolutePath(workingModel_relative); }

Path Directories::getShaderLibraryPath() { return getAbsolutePath(libraryShaderPath_relative); }
