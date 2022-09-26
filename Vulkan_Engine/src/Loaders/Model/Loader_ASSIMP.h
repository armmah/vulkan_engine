#pragma once
#include "pch.h"

#include <assimp/assimp/matrix4x4.h>
struct ASSIMP_API aiNode;

struct Mesh;
class Material;
struct Transform;
struct Renderer;
struct Path;

namespace Loader
{
	glm::mat4 convertToGLM(const aiMatrix4x4& src);
	void crawl(std::vector<Transform>& globalTransformCollection, std::unordered_map<int, size_t>& meshToTransform, aiNode* node, const aiMatrix4x4& parentMatrix, int depth);
	bool load_AssimpImplementation(std::vector<Mesh>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, std::vector<Transform>& transforms, const Path& fullPath);
}