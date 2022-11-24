#pragma once
#include "pch.h"
#include "Common.h"

class Material;
struct VkMesh;
struct VkGraphicsPipeline;
struct BoundsAABB;

enum VariantStateChange : uint32_t
{
	None = 0,

	Pipeline = 1,

	DescriptorSet_GlobalUBO = 5,
	DescriptorSet_CameraUBO = 6,

	DescriptorSet = 7
};

struct VkMaterialVariant
{
	VkMaterialVariant(const VkPipeline pipeline, const VkPipelineLayout pipelineLayout, const VkDescriptorSetLayout descriptorSetLayout, std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets);

	const VkPipeline getPipeline() const;
	const VkPipelineLayout getPipelineLayout() const;
	const VkDescriptorSet* getDescriptorSet(uint32_t frameNumber) const;
	const VkDescriptorSetLayout getDescriptorSetLayout() const;

	VariantStateChange compare(const VkMaterialVariant* other) const;

	void release(VkDevice device);

private:
	const VkPipeline m_pipeline;
	const VkPipelineLayout m_pipelineLayout;
	const VkDescriptorSetLayout m_descriptorSetLayout;

	std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT> m_descriptorSets;
};
