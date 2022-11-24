#include "pch.h"
#include "Texture.h"
#include "Material.h"
#include "dds_loader.h"

#include "VkTypes/InitializersUtility.h"
#include "Presentation/Device.h"
#include "FileManager/FileIO.h"

bool Texture::tryLoadSupportedFormat(Texture& texture, const std::string& path)
{
	if (!FileIO::fileExists(path))
		return false;

	if (FileIO::fileExists(path, ".dds"))
	{
		return ddsLoad(texture, path);
	}

	std::vector<std::string> stbi_supportedFormats = { ".png", ".jpg" };
	if (FileIO::fileExists(path, stbi_supportedFormats))
	{
		return stbiLoad(texture, path);
	}

	return false;
}

bool Texture::ddsLoad(Texture& texture, const std::string& path)
{
	auto tex = nv_dds::CDDSImage();
	tex.load(path, false);

	if (tex.is_valid())
	{
		VkFormat format;
		unsigned int width, height, channels;

		auto mipCount = static_cast<size_t>(tex.get_num_mipmaps());
		std::vector<LoadedTexture> textureMipchain;
		textureMipchain.reserve(mipCount + 1);

		width = tex.get_width();
		height = tex.get_height();
		channels = tex.get_components();

		if (width == 0 || height == 0 || channels == 0)
			return false;

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

		textureMipchain.push_back(LoadedTexture(tex.claim_data(), tex.get_size(), width, height));

		for (uint32_t mipIndex = 0; mipIndex < mipCount; mipIndex++)
		{
			auto& mip = tex.get_mipmap(mipIndex);
			auto* mipData = tex.claim_mipmap_data(mipIndex);
			textureMipchain.push_back(LoadedTexture(mipData, mip.get_size(), mip.get_width(), mip.get_height()));
		}

		texture.Init(std::move(textureMipchain), format, as_uint32(width), as_uint32(height), as_uint32(channels));
		return true;
	}
	return false;
}

bool Texture::stbiLoad(Texture& texture, const std::string& path)
{
	if (!FileIO::fileExists(path))
		return false;

	int width, height, channels;
	stbi_uc* const pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (width <= 0 || height <= 0 || channels <= 0 || !pixels)
	{
		printf("Could not load the image at %s.\n", path.c_str());
		return false;
	}

	std::vector<LoadedTexture> textureData;
	textureData.push_back(LoadedTexture(pixels, static_cast<size_t>(width) * static_cast<size_t>(height) * 4, width, height));

	texture.Init(std::move(textureData), VK_FORMAT_R8G8B8A8_SRGB, as_uint32(width), as_uint32(height), as_uint32(channels));
	return true;
}

void Texture::copyBufferToImage(const Presentation::Device* presentationDevice, VkBuffer buffer, VkImage image, const std::vector<MipDesc>& dimensions)
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
				region.imageSubresource.mipLevel = as_uint32(i);
				region.imageSubresource.baseArrayLayer = 0;
				region.imageSubresource.layerCount = 1;

				region.imageOffset = { 0, 0, 0 };
				region.imageExtent = { dimensions[i].width, dimensions[i].height, 1 };

				offsets += dimensions[i].imageByteSize;
			}

			vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, as_uint32(regions.size()), regions.data());
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

/*
VUID-VkDescriptorImageInfo-imageLayout-00344(ERROR / SPEC): msgNum: -564812795 - Validation Error: [ VUID-VkDescriptorImageInfo-imageLayout-00344 ] Object 0:
handle = 0x2afdd3d6060, type = VK_OBJECT_TYPE_COMMAND_BUFFER; | MessageID = 0xde55a405 | vkCmdDraw: Cannot use VkImage 0xa7c5450000000023[] (layer=0 mip=0)
with specific layout VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL that doesn't match the previous known layout VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL.
The Vulkan spec states: imageLayout must match the actual VkImageLayout of each subresource accessible from imageView at the time this descriptor is accessed
as defined by the image layout matching rules (https://vulkan.lunarg.com/doc/view/1.3.211.0/windows/1.3-extensions/vkspec.html#VUID-VkDescriptorImageInfo-imageLayout-00344)
	Objects: 1
		[0] 0x2afdd3d6060, type: 6, name: NULL

VUID-vkCmdDraw-None-02699(ERROR / SPEC): msgNum: 369680064 - Validation Error: [ VUID-vkCmdDraw-None-02699 ] Object 0: handle = 0x4b7df1000000002f,
type = VK_OBJECT_TYPE_DESCRIPTOR_SET; | MessageID = 0x1608dec0 | Descriptor set VkDescriptorSet 0x4b7df1000000002f[] encountered the following
validation error at vkCmdDraw time: Image layout specified at vkUpdateDescriptorSet* or vkCmdPushDescriptorSet* time doesn't match actual image layout
at time descriptor is used. See previous error callback for specific details. The Vulkan spec states: Descriptors in each bound descriptor set, specified
via vkCmdBindDescriptorSets, must be valid if they are statically used by the VkPipeline bound to the pipeline bind point used by this command (https://vulkan.lunarg.com/doc/view/1.3.211.0/windows/1.3-extensions/vkspec.html#VUID-vkCmdDraw-None-02699)
	Objects: 1
		[0] 0x4b7df1000000002f, type: 23, name: NULL
*/

void Texture::transitionImageLayout(const VkCommandBuffer cmd, const VkImage image, VkFormat format,
	VkImageLayout oldLayout, VkImageLayout newLayout, 
	uint32_t mipCount, VkImageAspectFlagBits subResImageAspect)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = subResImageAspect;
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
	else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		src.accesses = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		src.stages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		dst.accesses = VK_ACCESS_SHADER_READ_BIT;
		dst.stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
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
}

void Texture::transitionImageLayout(const Presentation::Device* presentationDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipCount)
{
	presentationDevice->submitImmediatelyAndWaitCompletion([=](VkCommandBuffer cmd)
		{
			Texture::transitionImageLayout(cmd, image, format, oldLayout, newLayout, mipCount);
		});
}
