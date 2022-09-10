#include "pch.h"
#include "Texture.h"
#include "VkTexture.h"
#include "Material.h"
#include "InitializersUtility.h"
#include "EngineCore/Texture.h"
#include "Presentation/Device.h"
#include "StagingBufferPool.h"

VkTexture2D::VkTexture2D(VkImage image, VmaAllocation memoryRange, VkImageView imageView, VkSampler sampler, uint32_t mipLevels) :
	VkTexture(image, memoryRange, imageView), sampler(sampler), mipLevels(mipLevels) { }

bool VkTexture2D::tryCreateTexture(UNQ<VkTexture2D>& tex, const TextureSource& texture, const Presentation::Device* presentationDevice, StagingBufferPool& stagingBufferPool)
{
	const auto* path = texture.path.c_str();

	auto format = texture.format;
	auto generateMips = texture.generateTheMips;

	std::vector<MipDesc> dimensions;
	uint32_t mipCount = 0u;

	Texture loadedTexture;
	StagingBufferPool::StgBuffer stagingBuffer;
	{
		if (!Texture::tryLoadSupportedFormat(loadedTexture, texture.path.value) || !loadedTexture.textureMipChain.size())
			return false;

		if (loadedTexture.format != format)
		{
			printf("The meta data was expecting format %i, but the texture '%s' had the format %i.\n", format, path, loadedTexture.format);
			format = loadedTexture.format;
		}

		mipCount = as_uint32(loadedTexture.textureMipChain.size());
		dimensions.reserve(mipCount);
		if (mipCount > 1u)
		{
			generateMips = false;
		}
		else if (generateMips)
		{
			mipCount = generateMips ? as_uint32(std::floor(std::log2(std::max(loadedTexture.width, loadedTexture.height)))) + 1 : 1u;
		}

		auto totalBufferSize = 0u;
		for (auto& mip : loadedTexture.textureMipChain)
		{
			totalBufferSize += mip.getByteSize();
			dimensions.push_back(mip.getDimensions());
		}

		if (!stagingBufferPool.claimAStagingBuffer(stagingBuffer, totalBufferSize))
		{
			printf("Could not allocate staging memory buffer for texture '%s'.\n", path);
			return false;
		}

		void* data;
		auto allocator = VkMemoryAllocator::getInstance()->m_allocator;
		vmaMapMemory(allocator, stagingBuffer.allocation, &data);
		size_t offset = 0;
		for (auto& mip : loadedTexture.textureMipChain)
		{
			mip.copyToMappedBuffer(data, offset);
			offset += mip.getByteSize();
		}
		vmaUnmapMemory(allocator, stagingBuffer.allocation);
	}
	loadedTexture.releasePixelData();

	VkImage image;
	VmaAllocation memoryRange;
	VkImageView imageView;
	VkSampler sampler;

	auto imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	if (generateMips)
	{
		imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	auto vmaci = VkMemoryAllocator::getInstance()->createAllocationDescriptor(VMA_MEMORY_USAGE_GPU_ONLY);
	if (!vkinit::Texture::createImage(image, memoryRange, vmaci, format, imageUsage, loadedTexture.width, loadedTexture.height, mipCount))
	{
		printf("Could not create image for texture '%s'.\n", path);
		return false;
	}

	Texture::transitionImageLayout(presentationDevice, image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipCount);
	Texture::copyBufferToImage(presentationDevice, stagingBuffer.buffer, image, dimensions);

	if (generateMips)
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

				int32_t mipWidth = loadedTexture.width,
					mipHeight = loadedTexture.height;
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
		printf("Could not create imageview or sampler for texture '%s'.\n", path);
		return false;
	}

	stagingBufferPool.freeBuffer(stagingBuffer);

	tex = MAKEUNQ<VkTexture2D>(image, memoryRange, imageView, sampler, mipCount);
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