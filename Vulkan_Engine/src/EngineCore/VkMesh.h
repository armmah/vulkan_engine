#pragma once
#include "pch.h"
#include "Common.h"
//#include "IndexAttributes.h"

struct VmaAllocator_T;
struct VertexAttributes;
struct IndexAttributes;

struct VkMesh
{
public:
	VkMesh();
	~VkMesh();

	void release(const VmaAllocator& allocator);

	UNQ<VertexAttributes> vAttributes;
	uint32_t vCount;

	UNQ<IndexAttributes> iAttributes;
	uint32_t iCount;
};
