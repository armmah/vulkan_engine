#pragma once
#include "pch.h"

struct Mesh;
class Material;
struct Transform;
struct Renderer;
namespace Loader { struct ModelLoaderOptions; }

namespace Loader
{
	bool loadOBJ_Implementation(std::vector<Mesh>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, std::vector<Transform>& transforms, 
		const Loader::ModelLoaderOptions& options);
}