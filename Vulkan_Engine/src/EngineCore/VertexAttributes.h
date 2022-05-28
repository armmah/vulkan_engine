#pragma once

#include "Common.h"
#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

struct VertexAttributes
{
public:
	VertexAttributes() = delete;
	VertexAttributes(std::vector<VkBuffer>& vertexBuffers, std::vector<VmaAllocation> vertexMemoryRanges, std::vector<VkDeviceSize>& vertexOffsets, uint32_t bindingOffset = 0, uint32_t bindingCount = 0) :
		buffers(std::move(vertexBuffers)), memoryRanges(std::move(vertexMemoryRanges)), offsets(std::move(vertexOffsets)), firstBinding(bindingOffset), bindingCount(bindingCount)
	{
		assert(buffers.size() == offsets.size() && "vertex buffer size should match the vertex offsets size.");

		if (bindingCount == 0)
		{
			this->bindingCount = as_uint32(buffers.size());
		}
	}

	void bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, buffers.data(), offsets.data());
	}

	void destroy(const VmaAllocator& allocator)
	{
		for (int i = 0; i < memoryRanges.size(); i++)
		{
			vmaDestroyBuffer(allocator, buffers[i], memoryRanges[i]);
		}
	}

private:
	std::vector<VkBuffer> buffers;
	std::vector<VmaAllocation> memoryRanges;
	std::vector<VkDeviceSize> offsets;
	uint32_t firstBinding;
	uint32_t bindingCount;
};
