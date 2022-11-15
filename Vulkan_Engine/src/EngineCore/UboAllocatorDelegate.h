#pragma once
#include "pch.h"
#include "VkTypes/PushConstantTypes.h"

struct BuffersUBO;

struct UBOAllocatorDelegate
{
	UBOAllocatorDelegate(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout) : m_device(device), m_descPool(pool), m_descriptorSetLayout(layout) { }

	bool invoke(BuffersUBO& buffer) const;

private:
	VkDevice m_device;
	VkDescriptorPool m_descPool;
	VkDescriptorSetLayout m_descriptorSetLayout;
};