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

	virtual bool isValid() const;
	virtual void release(VkDevice device);

	VkTexture(VkDevice device, MemAllocationInfo maci, VkFormat imageFormat, VkImageUsageFlags imageUsage, VkImageAspectFlags imageViewAspectFlags, VkExtent2D extent, uint32_t mipCount = 1);

	static VkTexture createTexture(VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlagBits usage, VkImageAspectFlagBits aspectFlags, bool isReadable = false, uint32_t mipCount = 1u)
	{
		const VmaMemoryUsage maciUsage = isReadable ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_GPU_ONLY;
		const VkMemoryPropertyFlagBits maciFlags = isReadable ? (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) : (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		const auto maci = VkMemoryAllocator::getInstance()->createAllocationDescriptor(maciUsage, maciFlags);
		auto extent = VkExtent2D{};
		extent.width = width;
		extent.height = height;

		return VkTexture(device, maci, format, usage, aspectFlags, extent);
	}

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