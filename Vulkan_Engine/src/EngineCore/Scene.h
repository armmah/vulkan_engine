#pragma once
#include "pch.h"
#include "Common.h"
#include "VkMesh.h"
#include "Mesh.h"

class VkMemoryAllocator;
struct VmaAllocator_T;

class Scene
{
public:
	const std::vector<UNQ<Mesh>>& getMeshes() const;
	const std::vector<UNQ<VkMesh>>& getGraphicsMeshes() const;

	bool load(const VmaAllocator& vmaAllocator);
	void release(const VmaAllocator& allocator);

private:
	std::vector<UNQ<Mesh>> meshes;
	std::vector<UNQ<VkMesh>> graphicsMeshes;
};
 