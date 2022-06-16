#pragma once

#include "pch.h"
#include "Common.h"
#include "VertexAttributes.h"
#include "IndexAttributes.h"

struct VmaAllocator_T;

struct VkMesh
{
public:
	VkMesh() : vAttributes(nullptr), vCount(0), iAttributes(nullptr), iCount(0) { }

	void release(const VmaAllocator& allocator);

	UNQ<VertexAttributes> vAttributes;
	uint32_t vCount;

	UNQ<IndexAttributes> iAttributes;
	uint32_t iCount;
};
