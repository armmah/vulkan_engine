#pragma once
#include "pch.h"

struct IndexAttributes
{
public:
	IndexAttributes() = delete;
	IndexAttributes(const VkBuffer& indexBuffer, const VmaAllocation& indexBufferMemory, uint32_t iCount, VkIndexType indexType, VkDeviceSize offset = 0);

	uint32_t getIndexCount() const { return iCount; }
	void bind(VkCommandBuffer commandBuffer) const;

	void destroy(const VmaAllocator& allocator);

private:
	VkBuffer buffer;
	VmaAllocation memoryRange;
	VkDeviceSize offset;
	VkIndexType indexType;

	uint32_t iCount;
};