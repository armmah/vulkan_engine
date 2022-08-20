#pragma once
#include "pch.h"

namespace Presentation
{
	class Device;
}

struct Texture
{
	static void copyBufferToImage(const Presentation::Device* presentationDevice, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

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

	static void transitionImageLayout(const Presentation::Device* presentationDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipCount);

	constexpr static float c_anisotropySamples = 4.0f;
};