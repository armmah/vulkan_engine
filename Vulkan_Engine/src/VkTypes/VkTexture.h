#pragma once
#include "pch.h"
#include "VkMemoryAllocator.h"

namespace Presentation
{
	class Device;
}

struct VkTexture
{
public:
	VkImage image;
	VmaAllocation memoryRange;

	VkImageView imageView;

	VkTexture(VkDevice device, MemAllocationInfo maci, VkFormat imageFormat, VkImageUsageFlags imageUsage, VkImageAspectFlags imageViewFormat, VkExtent2D extent);
	virtual void release(VkDevice device);

protected:
	VkTexture() : image(VK_NULL_HANDLE), memoryRange(VK_NULL_HANDLE), imageView(VK_NULL_HANDLE) { }
};

struct VkTexture2D : public VkTexture
{
	VkSampler sampler;
	uint32_t mipLevels;

	VkTexture2D(std::string path, const VmaAllocator& allocator, const Presentation::Device* presentationDevice);

	void release(VkDevice device) override;
};