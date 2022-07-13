#pragma once
#include "pch.h"
#include "Common.h"

class Material;

struct VkMaterialVariant
{
	VkDescriptorSetLayout descriptorSetLayout{};
	std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT> descriptorSets;

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	VkMaterialVariant(const Material* sourceMat, VkDevice device, VkDescriptorPool descriptorPool, bool hasDepthAttachment);

	void release(VkDevice device);

private:
	const Material* sourceMat;
};