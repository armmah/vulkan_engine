#pragma once
#include "pch.h"
#include "VkMemoryAllocator.h"

struct VkTexture
{
	VkImage image;
	VmaAllocation memoryRange;

	VkImageView imageView;

	virtual void release(VkDevice device)
	{
		vkDestroyImageView(device, imageView, nullptr);
		vmaDestroyImage(VkMemoryAllocator::getInstance()->m_allocator, image, memoryRange);
	}
};

struct VkTexture2D : public VkTexture
{
	VkSampler sampler;

	void release(VkDevice device) override
	{
		VkTexture::release(device);

		vkDestroySampler(device, sampler, nullptr);
	}
};