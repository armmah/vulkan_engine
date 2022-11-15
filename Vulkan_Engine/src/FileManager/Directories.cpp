#include "pch.h"
#include "Directories.h"

Path Directories::getAbsolutePath(const std::string& path) { return getApplicationPath().combine(path); }

Path Directories::getApplicationPath()
{
	return applicationPath;
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

Path Directories::syscall_GetApplicationPath()
{
	char pBuf[MAX_PATH];
	DWORD len = sizeof(pBuf);
	int bytes = GetModuleFileName(nullptr, pBuf, len);
	return bytes ? Path(pBuf) : Path();
}