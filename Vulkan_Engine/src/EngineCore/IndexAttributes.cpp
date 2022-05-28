#include "pch.h"
#include "IndexAttributes.h"

void IndexAttributes::bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

void IndexAttributes::destroy(const VmaAllocator& allocator)
{
	vmaDestroyBuffer(allocator, buffer, memoryRange);
}
