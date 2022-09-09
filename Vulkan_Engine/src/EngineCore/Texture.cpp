#include "pch.h"
#include "Texture.h"
#include "Material.h"
#include "dds_loader.h"

#include "VkTypes/InitializersUtility.h"
#include <Presentation/Device.h>

bool Texture::tryLoadSupportedFormat(std::vector<LoadedTexture>& textureMipchain, const std::string& path, VkFormat& format, int& width, int& height, int& channels)
{
	if (!fileExists(path))
		return false;

	std::vector<std::string> stbi_supportedFormats = { ".png", ".jpg" };
	if (fileExists(path, stbi_supportedFormats))
	{
		format = VK_FORMAT_R8G8B8A8_SRGB;
		return stbiLoad(textureMipchain, path, width, height, channels);
	}

	if (fileExists(path, ".dds"))
	{
		return ddsLoad(textureMipchain, path, format, width, height, channels);
	}

	return false;
}

bool Texture::ddsLoad(std::vector<LoadedTexture>& textureMipchain, const std::string& path, VkFormat& format, int& width, int& height, int& channels)
{
	auto tex = nv_dds::CDDSImage();
	tex.load(path, false);

	if (tex.is_valid())
	{
		switch (tex.get_format())
		{
		case 0x83F0:
			format = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
			break;
		case 0x83F1:
			format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
			break;
		case 0x83F2:
			format = VK_FORMAT_BC3_SRGB_BLOCK;
			break;
		case 0x83F3:
			format = VK_FORMAT_BC5_SNORM_BLOCK;
			break;

		default:
			printf("Unsupported texture format %i.\n", tex.get_format());
			return false;
		}

		auto mipCount = tex.get_num_mipmaps();
		textureMipchain.reserve(mipCount + 1);

		width = tex.get_width();
		height = tex.get_height();
		channels = tex.get_components();
		textureMipchain.push_back(LoadedTexture(tex.claim_data(), tex.get_size(), width, height));

		for (int mipIndex = 0; mipIndex < mipCount; mipIndex++)
		{
			auto& mip = tex.get_mipmap(mipIndex);
			auto* mipData = tex.claim_mipmap_data(mipIndex);
			textureMipchain.push_back(LoadedTexture(mipData, mip.get_size(), mip.get_width(), mip.get_height()));
		}
		return true;
	}
	return false;
}

bool Texture::stbiLoad(std::vector<LoadedTexture>& textureMipChain, const std::string& path, int& width, int& height, int& channels)
{
	if (!fileExists(path))
		return false;

	stbi_uc* const pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (width <= 0 || height <= 0 || channels <= 0 || !pixels)
	{
		printf("Could not load the image at %s.\n", path.c_str());
		return false;
	}

	textureMipChain.push_back(LoadedTexture(pixels, static_cast<size_t>(width) * static_cast<size_t>(height) * 4, width, height));
	return true;
}

void Texture::copyBufferToImage(const Presentation::Device* presentationDevice, VkBuffer buffer, VkImage image, const std::vector<VkExtent3D>& dimensions)
{
	presentationDevice->submitImmediatelyAndWaitCompletion([=](VkCommandBuffer cmd)
		{
			const auto count = dimensions.size();
			auto offsets = 0u;

			std::vector<VkBufferImageCopy> regions(count);
			for(size_t i = 0; i < count; i++)
			{
				regions[i]  = VkBufferImageCopy{};
				auto& region = regions[i];

				region.bufferOffset = offsets;
				region.bufferRowLength = 0;
				region.bufferImageHeight = 0;

				region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				region.imageSubresource.mipLevel = i;
				region.imageSubresource.baseArrayLayer = 0;
				region.imageSubresource.layerCount = 1;

				region.imageOffset = { 0, 0, 0 };
				region.imageExtent = { dimensions[i].width, dimensions[i].height, 1 };

				offsets += dimensions[i].depth;
			}

			vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions.size(), regions.data());
		});
}

struct _PipelineBarrierArg
{
	VkPipelineStageFlags stages;
	VkAccessFlags accesses;

	static void getStagingBufferState(_PipelineBarrierArg& src, _PipelineBarrierArg& dst)
	{
		src.accesses = 0;
		src.stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		dst.accesses = VK_ACCESS_TRANSFER_WRITE_BIT;
		dst.stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}

	static void getImageFragShaderState(_PipelineBarrierArg& src, _PipelineBarrierArg& dst)
	{
		src.accesses = VK_ACCESS_TRANSFER_WRITE_BIT;
		src.stages = VK_PIPELINE_STAGE_TRANSFER_BIT;

		dst.accesses = VK_ACCESS_SHADER_READ_BIT;
		dst.stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

};

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
