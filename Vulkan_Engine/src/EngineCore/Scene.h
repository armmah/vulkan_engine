#pragma once

#include "pch.h"
#include "Common.h"
#include "VkMesh.h"
#include "Mesh.h"

struct VmaAllocator_T;

class Scene
{
public:
	const std::vector<UNQ<Mesh>>& getMeshes() const { return meshes; }
	const std::vector<UNQ<VkMesh>>& getGraphicsMeshes() const { return graphicsMeshes; }

	bool load(const VmaAllocator& vmaAllocator);

	void release(const VmaAllocator& allocator);

private:
	std::vector<UNQ<Mesh>> meshes;
	std::vector<UNQ<VkMesh>> graphicsMeshes;
};
 