#include "pch.h"
#include "IndexAttributes.h"

IndexAttributes::IndexAttributes(const VkBuffer& indexBuffer, const VmaAllocation& indexBufferMemory, uint32_t iCount, VkIndexType indexType, VkDeviceSize offset)
	: buffer(indexBuffer), memoryRange(indexBufferMemory), iCount(iCount), offset(offset), indexType(indexType) { }

void IndexAttributes::bind(VkCommandBuffer commandBuffer) const
{
	vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

void IndexAttributes::destroy(const VmaAllocator& allocator)
{
	vmaDestroyBuffer(allocator, buffer, memoryRange);
}
