#pragma once

#include "Common.h"
#include "vk_mem_alloc.h"

#include "VertexAttributes.h"
#include "IndexAttributes.h"

struct VkMesh
{
public:
	VkMesh() : vAttributes(nullptr), vCount(0), iAttributes(nullptr), iCount(0) { }

	void release(const VmaAllocator& allocator)
	{
		vAttributes->destroy(allocator);
		iAttributes->destroy(allocator);
	}

	UNQ<VertexAttributes> vAttributes;
	uint32_t vCount;

	UNQ<IndexAttributes> iAttributes;
	uint32_t iCount;
};
