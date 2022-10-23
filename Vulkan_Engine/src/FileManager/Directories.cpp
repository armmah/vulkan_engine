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

Path Directories::getShaderLibraryPath() { return getAbsolutePath(libraryShaderPath_relative); }

std::vector<Path> Directories::getModels_IntelSponza()
{
	return { getAbsolutePath(workingModel_relative), getAbsolutePath(additiveModel_relative) };
}
std::vector<Path> Directories::getModels_DebrovicSponza()
{
	return { getAbsolutePath(DEBROVIC_SPONZA_OBJ) };
}
std::vector<Path> Directories::getModels_CrytekSponza()
{
	return { getAbsolutePath(CRYTEK_SPONZA_OBJ) };
}