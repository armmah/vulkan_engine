#pragma once
#include "pch.h"
#include "Common.h"
#include "VkMesh.h"
#include "Mesh.h"

class VkMemoryAllocator;
struct VmaAllocator_T;
class Material;

class Scene
{
public:
	const std::vector<UNQ<Mesh>>& getMeshes() const;
	const std::vector<UNQ<VkMesh>>& getGraphicsMeshes() const;

	bool load(const VmaAllocator& vmaAllocator);
	void release(const VmaAllocator& allocator);

	static bool tryLoadFromFile(std::vector<UNQ<Mesh>>& mesh, std::vector<UNQ<Material>>& materials, const std::string& path);

private:
	std::vector<UNQ<Mesh>> meshes;
	std::vector<UNQ<VkMesh>> graphicsMeshes;
};
 