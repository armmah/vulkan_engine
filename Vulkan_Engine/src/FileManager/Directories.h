#pragma once
#include "pch.h"
#include "Path.h"

#define EDITOR

class Directories
{
public:
	static Path getApplicationPath();;
	static Path getAbsolutePath(const std::string& path);
	
	static Path getWorkingDirectory();
	static Path getWorkingScene();
	static Path getWorkingModel();
	static std::vector<Path> getWorkingModels();
	static Path getShaderLibraryPath();

private:
	inline static std::string library_relative = "Resources/Library/";
	inline static std::string libraryShaderPath_relative = "Resources/Library/outputSPV/";

	inline static std::string CUBE_GLTF = "Resources/Other/gltf/Cube_khr.gltf";

	inline static std::string CRYTEK_SPONZA_OBJ = "Resources/classic_sponza/sponza.obj";
	inline static std::string CRYTEK_SPONZA_FBX = "Resources/classic_sponza/sponza.fbx";
	inline static std::string LIGHTING_SCENE = "Resources/classic_sponza/debrovic_sponza/sponza.obj";

	inline static std::string INTEL_SPONZA_FBX = "Resources/intel_sponza/NewSponza_Main_Yup_002.fbx";
	inline static std::string INTEL_SPONZA_GLTF = "Resources/intel_sponza/NewSponza_Main_glTF_002.gltf";
	inline static std::string INTEL_SPONZA_CURTAINS_GLTF = "Resources/intel_sponza/NewSponza_Curtains_glTF.gltf";

	inline static std::string workingSceneName = "Scene.binary";
	inline static std::string workingSceneDir_relative = "Resources/Serialized/";
	inline static std::string workingModel_relative = INTEL_SPONZA_GLTF;
	inline static std::string additiveModel_relative = INTEL_SPONZA_CURTAINS_GLTF;
};