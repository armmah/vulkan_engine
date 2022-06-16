#include "pch.h"
#include "VkMemoryAllocator.h"
#include "VkTypes/InitializersUtility.h"


VkMemoryAllocator::VkMemoryAllocator(VkInstance instance, VkPhysicalDevice hardware, VkDevice device)
{
	m_isInitialized = vkinit::MemoryBuffer::createVmaAllocator(m_allocator, instance, hardware, device);
	m_instance = this;
}

MemAllocationInfo VkMemoryAllocator::createAllocationDescriptor(VmaMemoryUsage usage, VmaAllocationCreateFlags flags) const
{
	MemAllocationInfo maci{};
	maci.allocator = &m_allocator;
	maci.usage = usage;
	maci.flags = flags;

	return maci;
}
