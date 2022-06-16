#include "pch.h"
#include "VkMesh.h"
#include "vk_mem_alloc.h"

#include "VertexAttributes.h"
#include "IndexAttributes.h"

void VkMesh::release(const VmaAllocator& allocator)
{
	vAttributes->destroy(allocator);
	iAttributes->destroy(allocator);
}
