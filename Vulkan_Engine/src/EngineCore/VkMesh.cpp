#include "pch.h"
#include "VkMesh.h"
#include "vk_mem_alloc.h"
#include "IndexAttributes.h"
#include "VertexAttributes.h"

VkMesh::VkMesh() : vAttributes(nullptr), vCount(0), iAttributes() { }
VkMesh::~VkMesh() { }

void VkMesh::release(const VmaAllocator& allocator)
{
	vAttributes->destroy(allocator);

	for (auto& iAttr : iAttributes)
	{
		iAttr.destroy(allocator);
	}
}
