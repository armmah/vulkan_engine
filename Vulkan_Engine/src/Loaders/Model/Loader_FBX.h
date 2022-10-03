#pragma once
#include "pch.h"

struct Mesh;
class Material;
struct Transform;
struct Renderer;
struct Path;

namespace Loader
{
	bool loadFBX_Implementation(std::vector<Mesh>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, std::vector<Transform>& transforms, const std::string& path, const std::string& name);
}