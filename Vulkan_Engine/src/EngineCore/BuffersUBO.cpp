#include "pch.h"
#include "BuffersUBO.h"
#include "VkTypes/InitializersUtility.h"

BuffersUBO::BuffersUBO() : descriptorSets(), buffers(), memoryRanges(), byteSize(0), m_isInitialized(false) { }

BuffersUBO::BuffersUBO(VkDescriptorSetLayout layout, uint32_t byteSize) : descriptorSets(), buffers(), memoryRanges(), byteSize(byteSize), m_isInitialized(false) { }

bool BuffersUBO::isInitialized() const { return m_isInitialized; }

bool BuffersUBO::allocate(VkDevice device, VkDescriptorPool descPool, VkDescriptorSetLayout layout)
{
	const auto& allocator = VkMemoryAllocator::getInstance()->m_allocator;
	for (size_t i = 0; i < buffers.size(); i++)
	{
		if (!vkinit::MemoryBuffer::allocateBufferAndMemory(buffers[i], memoryRanges[i], allocator, byteSize, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT))
			return false;
	}

	if (!vkinit::Descriptor::createDescriptorSets(descriptorSets, device, descPool, layout))
		return false;

	vkinit::Descriptor::updateDescriptorSets(descriptorSets, device, *this);

	return m_isInitialized = true;
}

BufferHandle BuffersUBO::getHandle(uint32_t frameNumber)
{
	const auto index = frameNumber % buffers.size();

	return BufferHandle(buffers[index], memoryRanges[index], descriptorSets[index]);
}

void BuffersUBO::release()
{
	const auto& allocator = VkMemoryAllocator::getInstance()->m_allocator;

	for (size_t i = 0; i < buffers.size(); i++)
	{
		vmaDestroyBuffer(allocator, buffers[i], memoryRanges[i]);
	}
}

BufferHandle::BufferHandle(VkBuffer buffer, VmaAllocation allocation, VkDescriptorSet descriptorSet)
	: buffer(buffer), allocation(allocation), descriptorSet(descriptorSet) { }

void BufferHandle::CopyData(void* source, size_t byteSize) const
{
	const auto& allocator = VkMemoryAllocator::getInstance()->m_allocator;

	void* data;
	vmaMapMemory(allocator, allocation, &data);
	memcpy(data, source, byteSize);
	vmaUnmapMemory(allocator, allocation);
}
