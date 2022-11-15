#include "pch.h"
#include "VertexAttributes.h"
#include "Common.h"

VertexAttributes::VertexAttributes(std::vector<VkBuffer>& vertexBuffers, std::vector<VmaAllocation> vertexMemoryRanges, std::vector<VkDeviceSize>& vertexOffsets, uint32_t bindingOffset, uint32_t bindingCount) :
	buffers(std::move(vertexBuffers)), memoryRanges(std::move(vertexMemoryRanges)), offsets(std::move(vertexOffsets)), firstBinding(bindingOffset), bindingCount(bindingCount)
{
	assert(buffers.size() == offsets.size() && "vertex buffer size should match the vertex offsets size.");

	if (bindingCount == 0)
	{
		this->bindingCount = as_uint32(buffers.size());
	}
}

void VertexAttributes::bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, buffers.data(), offsets.data());
}

void VertexAttributes::destroy(const VmaAllocator& allocator)
{
	for (int i = 0; i < memoryRanges.size(); i++)
	{
		vmaDestroyBuffer(allocator, buffers[i], memoryRanges[i]);
	}
}
