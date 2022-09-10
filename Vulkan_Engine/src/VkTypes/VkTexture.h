#pragma once
#include "pch.h"
#include "VkMemoryAllocator.h"
#include "Common.h"

namespace Presentation
{
	class Device;
}
class StagingBufferPool;
struct TextureSource;

struct VkTexture
{
public:
	VkImage image;
	VmaAllocation memoryRange;

	VkImageView imageView;

	virtual void release(VkDevice device);

	VkTexture(VkDevice device, MemAllocationInfo maci, VkFormat imageFormat, VkImageUsageFlags imageUsage, VkImageAspectFlags imageViewFormat, VkExtent2D extent, uint32_t mipCount = 1);
protected:
	VkTexture();
	VkTexture(VkImage image, VmaAllocation memoryRange, VkImageView imageView);
};

struct VkTexture2D : public VkTexture
{
	VkSampler sampler;
	uint32_t mipLevels;

	VkTexture2D(VkImage image, VmaAllocation memoryRange, VkImageView imageView, VkSampler sampler, uint32_t mipLevels);
	void release(VkDevice device) override;

	static bool tryCreateTexture(UNQ<VkTexture2D>& tex, const TextureSource& path, const Presentation::Device* presentationDevice, StagingBufferPool& stagingBufferPool);
};