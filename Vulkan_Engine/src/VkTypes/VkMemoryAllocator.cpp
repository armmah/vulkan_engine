#include "pch.h"
#include "VkMemoryAllocator.h"
#include "VkTypes/InitializersUtility.h"

bool VkMemoryAllocator::isInitialized() const { return m_isInitialized; }

VkMemoryAllocator::VkMemoryAllocator(VkInstance instance, VkPhysicalDevice hardware, VkDevice device)
{
	m_isInitialized = vkinit::MemoryBuffer::createVmaAllocator(m_allocator, instance, hardware, device);
	m_instance = this;
}

void VkMemoryAllocator::release() { vmaDestroyAllocator(m_allocator); }

MemAllocationInfo VkMemoryAllocator::createAllocationDescriptor(VmaMemoryUsage usage, VmaAllocationCreateFlags flags) const
{
	MemAllocationInfo maci{};
	maci.allocator = &m_allocator;
	maci.usage = usage;
	maci.flags = flags;

	return maci;
}

VmaAllocationCreateInfo MemAllocationInfo::getAllocationCreateInfo() const
{
	VmaAllocationCreateInfo aci = {};
	aci.usage = usage;
	aci.flags = flags;

	return aci;
}
