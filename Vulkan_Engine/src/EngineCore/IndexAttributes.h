#pragma once

#include "pch.h"
#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

struct IndexAttributes
{
public:
	IndexAttributes() = delete;
	IndexAttributes(const VkBuffer& indexBuffer, const VmaAllocation& indexBufferMemory, VkIndexType indexType, VkDeviceSize offset = 0)
		: buffer(indexBuffer), memoryRange(indexBufferMemory), offset(offset), indexType(indexType) { }

	void bind(VkCommandBuffer commandBuffer);

	void destroy(const VmaAllocator& allocator);

private:
	VkBuffer buffer;
	VmaAllocation memoryRange;
	VkDeviceSize offset;
	VkIndexType indexType;
};