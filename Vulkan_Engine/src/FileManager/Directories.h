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

	static std::vector<Path> getModels_IntelSponza();
	static std::vector<Path> getModels_DebrovicSponza();
	static std::vector<Path> getModels_CrytekSponza();

	static Path getShaderLibraryPath();

	static bool isBinary(const Path& scenePath) { return scenePath.matchesExtension(sceneFileExtension); }

	static Path getBinaryTargetPath(const Path& modelPath)
	{
		auto fileName = modelPath.getFileName(false) + sceneFileExtension;
		return getAbsolutePath(workingSceneDir_relative).combine(fileName);
	}

	static bool tryGetBinaryIfExists(Path& binaryPath, const Path& modelPath)
	{
		binaryPath = getBinaryTargetPath(modelPath);
		return binaryPath.fileExists();
	}

private:
	inline static std::string sceneFileExtension = ".binary";
	inline static std::string library_relative = "Resources/Library/";
	inline static std::string libraryShaderPath_relative = "Resources/Library/outputSPV/";

	inline static std::string CUBE_GLTF = "Resources/Other/gltf/Cube_khr.gltf";

	inline static std::string CRYTEK_SPONZA_OBJ = "Resources/classic_sponza/crytekSponza.obj";
	inline static std::string CRYTEK_SPONZA_FBX = "Resources/classic_sponza/crytekSponza.fbx";
	inline static std::string DEBROVIC_SPONZA_OBJ = "Resources/classic_sponza/debrovic_sponza/debrovicSponza.obj";

	inline static std::string INTEL_SPONZA_FBX = "Resources/intel_sponza/NewSponza_Main_Yup_002.fbx";
	inline static std::string INTEL_SPONZA_GLTF = "Resources/intel_sponza/NewSponza_Main_glTF_002.gltf";
	inline static std::string INTEL_SPONZA_CURTAINS_GLTF = "Resources/intel_sponza/NewSponza_Curtains_glTF.gltf";

	inline static std::string workingSceneName = "Scene.binary";
	inline static std::string workingSceneDir_relative = "Resources/Serialized/";
	inline static std::string workingModel_relative = INTEL_SPONZA_GLTF;
	inline static std::string additiveModel_relative = INTEL_SPONZA_CURTAINS_GLTF;
};