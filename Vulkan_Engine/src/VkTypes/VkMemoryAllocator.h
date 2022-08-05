#pragma once
#include "pch.h"
#include "Interfaces/IRequireInitialization.h"

struct MemAllocationInfo
{
	const VmaAllocator* allocator;
	VmaMemoryUsage usage;
	VmaAllocationCreateFlags flags;

	VmaAllocationCreateInfo getAllocationCreateInfo() const
	{
		VmaAllocationCreateInfo aci = {};
		aci.usage = usage;
		aci.flags = flags;

		return aci;
	}
};

class VkMemoryAllocator : IRequireInitialization
{
public:
	const static VkMemoryAllocator* getInstance() { return m_instance; }
	virtual bool isInitialized() const override { return m_isInitialized; }

	VkMemoryAllocator(VkInstance instance, VkPhysicalDevice hardware, VkDevice device);
	void release();

	MemAllocationInfo createAllocationDescriptor(VmaMemoryUsage usage, VmaAllocationCreateFlags flags = 0) const;

	VmaAllocator m_allocator;
private:
	inline const static VkMemoryAllocator* m_instance = nullptr;
	bool m_isInitialized;
};