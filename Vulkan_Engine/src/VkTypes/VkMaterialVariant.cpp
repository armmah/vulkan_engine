#include "pch.h"
#include "VkMaterialVariant.h"
#include "InitializersUtility.h"
#include "Mesh.h"
#include "PresentationTarget.h"
#include "Material.h"
#include "Engine/Bitmask.h"

VkMaterialVariant::VkMaterialVariant(const VkPipeline pipeline, const VkPipelineLayout pipelineLayout, const VkDescriptorSetLayout descriptorSetLayout, std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets)
	: m_pipeline(pipeline), m_pipelineLayout(pipelineLayout), m_descriptorSetLayout(descriptorSetLayout), m_descriptorSets(descriptorSets) { }

const VkPipeline VkMaterialVariant::getPipeline() const { return m_pipeline; }
const VkPipelineLayout VkMaterialVariant::getPipelineLayout() const { return m_pipelineLayout; }
const VkDescriptorSetLayout VkMaterialVariant::getDescriptorSetLayout() const { return m_descriptorSetLayout; }
const VkDescriptorSet* VkMaterialVariant::getDescriptorSet(uint32_t frameNumber) const { return &m_descriptorSets[frameNumber % SWAPCHAIN_IMAGE_COUNT]; }

VariantStateChange VkMaterialVariant::compare(const VkMaterialVariant* other) const
{
	VariantStateChange state = VariantStateChange::None;

	if (other == nullptr)
	{
		return static_cast<VariantStateChange>(
			VariantStateChange::Pipeline | VariantStateChange::DescriptorSet_GlobalUBO |
			VariantStateChange::DescriptorSet_CameraUBO | VariantStateChange::DescriptorSet
			);
	}

	state = bitFlagAppendIf(state, m_pipeline != other->m_pipeline, VariantStateChange::Pipeline);
	state = bitFlagAppendIf(state, &m_descriptorSets != &other->m_descriptorSets, VariantStateChange::DescriptorSet);

	return state;
}

void VkMaterialVariant::release(VkDevice device) { }