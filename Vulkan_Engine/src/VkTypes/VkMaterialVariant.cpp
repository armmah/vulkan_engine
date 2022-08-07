#include "pch.h"
#include "VkMaterialVariant.h"
#include "InitializersUtility.h"
#include "Mesh.h"
#include "PresentationTarget.h"
#include "Material.h"

VkMaterialVariant::VkMaterialVariant(const VkPipeline pipeline, const VkPipelineLayout pipelineLayout, const VkDescriptorSetLayout descriptorSetLayout, std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets)
	: pipeline(pipeline), pipelineLayout(pipelineLayout), descriptorSetLayout(descriptorSetLayout), descriptorSets(descriptorSets) { }

const VkPipeline VkMaterialVariant::getPipeline() const { return pipeline; }
const VkPipelineLayout VkMaterialVariant::getPipelineLayout() const { return pipelineLayout; }
const VkDescriptorSetLayout VkMaterialVariant::getDescriptorSetLayout() const { return descriptorSetLayout; }
const VkDescriptorSet* VkMaterialVariant::getDescriptorSet(uint32_t frameNumber) const { return &descriptorSets[frameNumber % SWAPCHAIN_IMAGE_COUNT]; }

VariantStateChange VkMaterialVariant::compare(const VkMaterialVariant* other) const
{
	VariantStateChange state = VariantStateChange::None;

	if (other == nullptr)
		return static_cast<VariantStateChange>(VariantStateChange::Pipeline | VariantStateChange::DescriptorSet);

	state = bitFlagAppendIf(state, pipeline != other->pipeline, VariantStateChange::Pipeline);
	state = bitFlagAppendIf(state, &descriptorSets != &other->descriptorSets, VariantStateChange::DescriptorSet);

	//state = static_cast<VariantStateChange>(state | (graphicsPipeline == other->graphicsPipeline) << VariantStateChange::Pipeline);
	//state = static_cast<VariantStateChange>(state | (&descriptorSets == &other->descriptorSets) << VariantStateChange::DescriptorSet);

	return state;
}

void VkMaterialVariant::release(VkDevice device) { }
