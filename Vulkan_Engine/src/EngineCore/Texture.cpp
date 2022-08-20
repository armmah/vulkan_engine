#include "pch.h"
#include "Texture.h"

#include "VkTypes/InitializersUtility.h"
#include <Presentation/Device.h>

void Texture::copyBufferToImage(const Presentation::Device* presentationDevice, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
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

void Texture::transitionImageLayout(const Presentation::Device* presentationDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipCount)
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
			barrier.subresourceRange.levelCount = mipCount;
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
