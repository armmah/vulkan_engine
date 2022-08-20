#include "pch.h"
#include "VkTexture.h"
#include "InitializersUtility.h"
#include <EngineCore/Texture.h>
#include "Presentation/Device.h"

VkTexture2D::VkTexture2D(VkImage image, VmaAllocation memoryRange, VkImageView imageView, VkSampler sampler, uint32_t mipLevels) :
	VkTexture(image, memoryRange, imageView), sampler(sampler), mipLevels(mipLevels) { }

bool VkTexture2D::tryCreateTexture(UNQ<VkTexture2D>& tex, std::string path, const Presentation::Device* presentationDevice, bool generateTheMips)
{
	if (!fileExists(path))
		return false;

	auto allocator = VkMemoryAllocator::getInstance()->m_allocator;

	int width, height, channels;
	stbi_uc* const pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	size_t imageSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
	auto mipCount = generateTheMips ? static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1 : 1u;

	if (width <= 0 || height <= 0 || imageSize <= 0 || !pixels)
	{
		printf("Could not load the image at %s.\n", path.c_str());
		return false;
	}

	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	if (!vkinit::MemoryBuffer::allocateBufferAndMemory(stagingBuffer, stagingAllocation, allocator, as_uint32(imageSize), VMA_MEMORY_USAGE_CPU_TO_GPU, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
	{
		printf("Could not allocate staging memory buffer for texture '%s'.\n", path.c_str());
		return false;
	}

	void* data;
	vmaMapMemory(allocator, stagingAllocation, &data);
	memcpy(data, pixels, imageSize);
	vmaUnmapMemory(allocator, stagingAllocation);
	stbi_image_free(static_cast<void*>(pixels));

	VkImage image;
	VmaAllocation memoryRange;
	VkImageView imageView;
	VkSampler sampler;

	auto format = VK_FORMAT_R8G8B8A8_SRGB;

	auto imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	if (generateTheMips)
	{
		imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	auto vmaci = VkMemoryAllocator::getInstance()->createAllocationDescriptor(VMA_MEMORY_USAGE_GPU_ONLY);
	if (!vkinit::Texture::createImage(image, memoryRange, vmaci, format, imageUsage, as_uint32(width), as_uint32(height), mipCount))
	{
		printf("Could not create image for texture '%s'.\n", path.c_str());
		return false;
	}

	Texture::transitionImageLayout(presentationDevice, image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipCount);
	Texture::copyBufferToImage(presentationDevice, stagingBuffer, image, as_uint32(width), as_uint32(height));

	if (generateTheMips)
	{
		presentationDevice->submitImmediatelyAndWaitCompletion([=](VkCommandBuffer commandBuffer)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = image;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.subresourceRange.levelCount = 1;

			int32_t mipWidth = width,
				mipHeight = height;
			for (uint32_t i = 1; i < mipCount; i++) 
			{
				// Transfer layout for previous mip
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier);

				VkImageBlit blit{};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;

				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { std::max(mipWidth >> 1, 1), std::max(mipHeight >> 1, 1), 1 };
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;

				vkCmdBlitImage(commandBuffer,
					image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &blit,
					VK_FILTER_LINEAR);

				// Transition to shader read for previous mip, we are done with it.
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier);

				mipWidth = std::max(mipWidth >> 1, 1);
				mipHeight = std::max(mipHeight >> 1, 1);
			}

			// The last mip (it isn't sampled from, so doesn't transition)
			barrier.subresourceRange.baseMipLevel = mipCount - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);
		});
	}
	else
	{
		// When generating the mipmaps, we do the transition on the whole image, after all mips are generated.
		Texture::transitionImageLayout(presentationDevice, image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipCount);
	}

	if (!vkinit::Texture::createTextureImageView(imageView, presentationDevice->getDevice(), image, format, mipCount) ||
		!vkinit::Texture::createTextureSampler(sampler, presentationDevice->getDevice(), mipCount, true, VK_SAMPLER_ADDRESS_MODE_REPEAT, Texture::c_anisotropySamples))
	{
		printf("Could not create imageview or sampler for texture '%s'.\n", path.c_str());
		return false;
	}

	vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

	tex = std::unique_ptr<VkTexture2D>(new VkTexture2D(image, memoryRange, imageView, sampler, mipCount));
	return true;
}

void VkTexture2D::release(VkDevice device)
{
	VkTexture::release(device);

	vkDestroySampler(device, sampler, nullptr);
}


VkTexture::VkTexture() : image(VK_NULL_HANDLE), memoryRange(VK_NULL_HANDLE), imageView(VK_NULL_HANDLE) { }
VkTexture::VkTexture(VkImage image, VmaAllocation memoryRange, VkImageView imageView) : image(image), memoryRange(memoryRange), imageView(imageView) { }
VkTexture::VkTexture(VkDevice device, MemAllocationInfo maci, VkFormat imageFormat, VkImageUsageFlags imageUsage, VkImageAspectFlags imageViewFormat, VkExtent2D extent, uint32_t mipCount)
{
	vkinit::Texture::createImage(image, memoryRange, maci, imageFormat, imageUsage, extent.width, extent.height, mipCount);
	vkinit::Texture::createTextureImageView(imageView, device, image, imageFormat, mipCount, imageViewFormat);
}

void VkTexture::release(VkDevice device)
{
	vkDestroyImageView(device, imageView, nullptr);
	vmaDestroyImage(VkMemoryAllocator::getInstance()->m_allocator, image, memoryRange);
}