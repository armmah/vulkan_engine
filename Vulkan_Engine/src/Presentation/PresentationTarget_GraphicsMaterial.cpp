#include "pch.h"
#include "PresentationTarget.h"
#include "Mesh.h"
#include "Material.h"
#include "VkTypes/VkShader.h"
#include "VkTypes/PipelineConstructor.h"

namespace Presentation
{
	bool PresentationTarget::createPipelineIfNotExist(VkGraphicsPipeline& graphicsPipeline, const VkPipelineLayout pipelineLayout,
		const VkDevice device, const VkShader* shader, const VkRenderPass renderPass, VkExtent2D extent)
	{
		if (!m_globalPipelineState->tryGetGraphicsPipelineFor(shader, graphicsPipeline))
		{
			graphicsPipeline.m_pipelineLayout = pipelineLayout;
			if (!PipelineConstruction::createPipeline(graphicsPipeline.m_pipeline, pipelineLayout, device,
				renderPass, extent, *shader, &Mesh::defaultMeshDescriptor,
				PipelineConstruction::FaceCulling::Back, hasDepthAttachement()))
				return false;

			m_globalPipelineState->insertGraphicsPipelineFor(shader, graphicsPipeline);
		}

		return true;
	}

	bool PresentationTarget::createGraphicsMaterial(UNQ<VkMaterial>& material, const VkDevice device, const VkDescriptorPool descPool, const VkShader* shader, const VkTexture2D* texture)
	{
		VkDescriptorSetLayout descriptorSetLayout = m_globalPipelineState->getDescriptorSetLayout(PipelineDescriptor::BindingSlots::Textures);

		std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT> descriptorSets;
		vkinit::Descriptor::createDescriptorSets(descriptorSets, device, descPool, descriptorSetLayout, *texture);

		VkGraphicsPipeline graphicsPipeline;
		if (!createPipelineIfNotExist(graphicsPipeline, m_globalPipelineState->getForwardPipelineLayout(), device, shader, getRenderPass(), getSwapchainExtent()))
			return false;

		material = MAKEUNQ<VkMaterial>(*shader, *texture,
			graphicsPipeline.m_pipeline, graphicsPipeline.m_pipelineLayout,
			descriptorSetLayout, descriptorSets);

		return true;
	}
}