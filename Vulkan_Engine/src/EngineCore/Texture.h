#pragma once
#include <vk_types.h>
#include <Presentation/Device.h>

struct Texture
{
	static void copyBufferToImage(const Presentation::Device* presentationDevice, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		presentationDevice->submitImmediatelyAndWaitCompletion([=](VkCommandBuffer cmd)
		{
			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height, 1 };

			vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region);
		});
	}

	struct _PipelineBarrierArg
	{
		VkPipelineStageFlags stages;
		VkAccessFlags accesses;

		static void getStagingBufferState(_PipelineBarrierArg& src, _PipelineBarrierArg& dst)
		{
			src.stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			src.accesses = 0;

			dst.stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dst.accesses = VK_ACCESS_TRANSFER_WRITE_BIT;
		}

		static void getImageFragShaderState(_PipelineBarrierArg& src, _PipelineBarrierArg& dst)
		{
			src.accesses = VK_ACCESS_TRANSFER_WRITE_BIT;
			src.stages = VK_PIPELINE_STAGE_TRANSFER_BIT;

			dst.accesses = VK_ACCESS_SHADER_READ_BIT;
			dst.stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
	};

	static void transitionImageLayout(const Presentation::Device* presentationDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		presentationDevice->submitImmediatelyAndWaitCompletion([=](VkCommandBuffer cmd)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;


			// To do - refactor this into more robust.
			_PipelineBarrierArg src, dst;
			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				_PipelineBarrierArg::getStagingBufferState(src, dst);
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				_PipelineBarrierArg::getImageFragShaderState(src, dst);
			}
			else
			{
				throw std::invalid_argument("unsupported layout transition!");
			}

			barrier.srcAccessMask = src.accesses;
			barrier.dstAccessMask = dst.accesses;

			vkCmdPipelineBarrier(
				cmd,
				src.stages,
				dst.stages,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		});
	}

	static bool loadImage(VkTexture& textureResource, std::string path, const VmaAllocator& allocator, const Presentation::Device* presentationDevice)
	{
		int width, height, channels;
		stbi_uc* const pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		VkDeviceSize imageSize = width * height * 4;

		if (imageSize <= 0 || !pixels)
		{
			printf("Could not load the image at %s.\n", path.c_str());
			return false;
		}

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;
		if (!vkinit::MemoryBuffer::allocateBufferAndMemory(stagingBuffer, stagingAllocation, allocator, as_uint32(imageSize), VMA_MEMORY_USAGE_CPU_TO_GPU, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
			return false;

		void* data;
		vmaMapMemory(allocator, stagingAllocation, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vmaUnmapMemory(allocator, stagingAllocation);
		stbi_image_free(static_cast<void*>(pixels));

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(width);
		imageInfo.extent.height = static_cast<uint32_t>(height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		VmaAllocationCreateInfo dimg_allocinfo = {};
		dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		if (vmaCreateImage(allocator, &imageInfo, &dimg_allocinfo, &textureResource.image, &textureResource.memoryRange, nullptr) != VK_SUCCESS)
			return false;

		transitionImageLayout(presentationDevice, textureResource.image, imageInfo.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(presentationDevice, stagingBuffer, textureResource.image, as_uint32(width), as_uint32(height));

		transitionImageLayout(presentationDevice, textureResource.image, imageInfo.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkinit::Texture::createTextureImageView(textureResource.imageView, presentationDevice->getDevice(), textureResource.image, imageInfo.format);
		vkinit::Texture::createTextureSampler(textureResource.sampler, presentationDevice->getDevice(), true, VK_SAMPLER_ADDRESS_MODE_REPEAT, 4.0f);

		vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
		return true;
	}
};