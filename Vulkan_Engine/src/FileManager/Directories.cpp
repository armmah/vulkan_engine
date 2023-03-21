#include "pch.h"
#include "Directories.h"
#include "Loaders/Model/ModelLoaderOptions.h"

Path Directories::getAbsolutePath(const std::string& path) { return getApplicationPath().combine(path); }

Path Directories::getApplicationPath()
{
	return applicationPath;
}

bool Directories::isValidWorkingDirectory(const Path& path) { return path.combine(workingSceneDir_relative).fileExists(); }

Path Directories::getWorkingDirectory() { return getAbsolutePath(workingSceneDir_relative); }

Path Directories::getShaderLibraryPath() { return getAbsolutePath(libraryShaderPath_relative); }

std::vector<Loader::ModelLoaderOptions> Directories::getModels_IntelSponza()
{
	return 
	{ 
		Loader::ModelLoaderOptions(getAbsolutePath(workingModel_relative), 1.0f),
		Loader::ModelLoaderOptions(getAbsolutePath(additiveModel_relative), 1.0f)
	};
}
std::vector<Loader::ModelLoaderOptions> Directories::getModels_DebrovicSponza()
{
	return 
	{ 
		Loader::ModelLoaderOptions(getAbsolutePath(DEBROVIC_SPONZA_OBJ), 1.0f)
	};
}
std::vector<Loader::ModelLoaderOptions> Directories::getModels_CrytekSponza()
{
	return 
	{ 
		Loader::ModelLoaderOptions(getAbsolutePath(CRYTEK_SPONZA_OBJ), 0.01f)
	};
}

bool Directories::isBinary(const Path& scenePath) { return scenePath.matchesExtension(sceneFileExtension); }

Path Directories::getBinaryTargetPath(const Path& modelPath)
{
	auto fileName = modelPath.getFileName(false) + sceneFileExtension;
	return getAbsolutePath(workingSceneDir_relative).combine(fileName);
}

bool Directories::tryGetBinaryIfExists(Path& binaryPath, const Path& modelPath)
{
	binaryPath = getBinaryTargetPath(modelPath);
	return binaryPath.fileExists();
}

Path Directories::syscall_GetApplicationPath()
{
	char pBuf[MAX_PATH]{};
	DWORD len = sizeof(pBuf);
	int bytes = GetModuleFileNameA(nullptr, pBuf, len);
	return bytes ? Path(pBuf) : Path();
}