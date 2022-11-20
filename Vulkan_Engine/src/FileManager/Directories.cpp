#include "pch.h"
#include "Directories.h"
#include "Loaders/Model/ModelLoaderOptions.h"

Path Directories::getAbsolutePath(const std::string& path) { return getApplicationPath().combine(path); }

Path Directories::getApplicationPath()
{
	return applicationPath;
}

Path Directories::getWorkingDirectory() { return getAbsolutePath(workingSceneDir_relative); }

Path Directories::getShaderLibraryPath() { return getAbsolutePath(libraryShaderPath_relative); }

std::vector<Loader::ModelLoaderOptions> Directories::getModels_IntelSponza()
{
	return 
	{ 
		Loader::ModelLoaderOptions(getAbsolutePath(workingModel_relative), 1.0),
		Loader::ModelLoaderOptions(getAbsolutePath(additiveModel_relative), 1.0)
	};
}
std::vector<Loader::ModelLoaderOptions> Directories::getModels_DebrovicSponza()
{
	return 
	{ 
		Loader::ModelLoaderOptions(getAbsolutePath(DEBROVIC_SPONZA_OBJ), 1.0)
	};
}
std::vector<Loader::ModelLoaderOptions> Directories::getModels_CrytekSponza()
{
	return 
	{ 
		Loader::ModelLoaderOptions(getAbsolutePath(CRYTEK_SPONZA_OBJ), 0.01)
	};
}

Path Directories::syscall_GetApplicationPath()
{
	char pBuf[MAX_PATH];
	DWORD len = sizeof(pBuf);
	int bytes = GetModuleFileName(nullptr, pBuf, len);
	return bytes ? Path(pBuf) : Path();
}