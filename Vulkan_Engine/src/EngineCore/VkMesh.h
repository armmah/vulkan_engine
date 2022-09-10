#pragma once
#include "pch.h"
#include "Common.h"
#include "IndexAttributes.h"

struct VmaAllocator_T;
struct VertexAttributes;

struct VkMesh
{
public:
	VkMesh();
	VkMesh(VkMesh&& fwdRef) noexcept;
	~VkMesh();

	void release(const VmaAllocator& allocator);

	UNQ<VertexAttributes> vAttributes;
	uint32_t vCount;
	
	std::vector<IndexAttributes> iAttributes;
};
