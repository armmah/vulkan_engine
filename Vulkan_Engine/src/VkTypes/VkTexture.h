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
struct Texture;

struct VkTexture
{
public:
	VkImage image;
	VmaAllocation memoryRange;

	VkImageView imageView;

	virtual bool isValid() const;
	virtual void release(VkDevice device);

	VkTexture(VkDevice device, MemAllocationInfo maci, VkFormat imageFormat, VkImageUsageFlags imageUsage, VkImageAspectFlags imageViewAspectFlags, VkExtent2D extent, uint32_t mipCount = 1);

	static VkTexture createTexture(VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlagBits usage, VkImageAspectFlagBits aspectFlags, bool isReadable = false, uint32_t mipCount = 1u);

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
	
	static bool tryCreateTexture(UNQ<VkTexture2D>& tex, const TextureSource& texture, const Presentation::Device* presentationDevice, StagingBufferPool& stagingBufferPool);
	static bool tryCreateTexture(UNQ<VkTexture2D>& tex, const Texture& loadedTexture, const Presentation::Device* presentationDevice, StagingBufferPool& stagingBufferPool, bool generateMips = false);
	static VkTexture2D createTexture(VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlagBits usage, VkImageAspectFlagBits aspectFlags, bool isReadable = false, uint32_t mipCount = 1u);
};