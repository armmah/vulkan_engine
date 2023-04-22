#pragma once
#include "pch.h"

struct IndexAttributes
{
public:
	IndexAttributes() = delete;
	IndexAttributes(VkBuffer indexBuffer, VmaAllocation indexBufferMemory, uint32_t iCount, VkIndexType indexType, VkDeviceSize offset = 0);

	uint32_t getIndexCount() const { return iCount; }
	void bind(VkCommandBuffer commandBuffer) const;

	void destroy(VmaAllocator allocator);

private:
	VkBuffer buffer;
	VmaAllocation memoryRange;
	VkDeviceSize offset;
	VkIndexType indexType;

	uint32_t iCount;
};