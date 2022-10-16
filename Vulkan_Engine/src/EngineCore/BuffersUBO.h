#pragma once
#include "pch.h"
#include "Interfaces/IRequireInitialization.h"
#include "VkTypes/VkMemoryAllocator.h"

struct BufferHandle
{
	BufferHandle(VkBuffer buffer, VmaAllocation allocation, VkDescriptorSet descriptorSet);

	void CopyData(void* source, size_t byteSize) const;

	VkDescriptorSet descriptorSet;

	VkBuffer buffer;
	VmaAllocation allocation;
};

struct BuffersUBO : IRequireInitialization
{
	std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT> descriptorSets;

	std::array<VkBuffer, SWAPCHAIN_IMAGE_COUNT> buffers;
	std::array<VmaAllocation, SWAPCHAIN_IMAGE_COUNT> memoryRanges;
	uint32_t byteSize;
	bool m_isInitialized = false;

	BuffersUBO();
	BuffersUBO(VkDescriptorSetLayout layout, uint32_t byteSize);
	bool isInitialized() const override;

	bool allocate(VkDevice device, VkDescriptorPool descPool, VkDescriptorSetLayout layout);

	BufferHandle getHandle(uint32_t frameNumber);

	void release();
};
