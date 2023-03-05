#include "pch.h"
#include "DebugPass.h"
#include "VkTypes/PipelineConstructor.h"
#include "DescriptorPoolManager.h"
#include "Material.h"
#include "Presentation/PresentationTarget.h"
#include "PipelineBinding.h"
#include "VkTypes/VkTexture.h"

namespace Presentation
{
	DebugPass::DebugPass() : Pass(false), m_debugQuad(), m_isInitialized(), m_shadowmapDescriptorSet(), m_shadowmapSampler() { }

	DebugPass::~DebugPass() { }

	DebugPass::DebugPass(PresentationTarget& target, VkDevice device, const VkShader* shader, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, VkExtent2D extent, const VkTexture2D& displayTexture)
		: Pass(false), m_debugQuad(), m_isInitialized(), m_shadowmapDescriptorSet(), m_shadowmapSampler()
	{
		m_debugQuad.m_pipelineLayout = pipelineLayout;
		if (PipelineConstruction::createPipeline(m_debugQuad.m_pipeline, pipelineLayout, device,
			renderPass, extent, *shader, nullptr, PipelineConstruction::FaceCulling::None, true))
		{
			target.m_globalPipelineState->insertGraphicsPipelineFor(shader, m_debugQuad);
		}
		else
		{
			printf("Could not create pipeline for the debug quad shader.\n");
			return;
		}

		const auto pool = DescriptorPoolManager::getInstance()->createNewPool(3u);
		VkDescriptorSetLayout descriptorSetLayout = target.m_globalPipelineState->getDescriptorSetLayout(PipelineDescriptor::BindingSlots::Shadowmap);

		m_isInitialized = vkinit::Texture::createTextureSampler(m_shadowmapSampler, device, 1u, true, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) &&
			vkinit::Descriptor::createDescriptorSets(m_shadowmapDescriptorSet, device, pool, descriptorSetLayout, displayTexture.imageView, m_shadowmapSampler);

		if (!m_isInitialized)
		{
			printf("Was not able to initialize Debug Pass.\n");
			setActive(false);
		}
	}

	bool DebugPass::isInitialized() const { return m_isInitialized; }
	
	const VkPipelineLayout DebugPass::getPipelineLayout() const { return m_debugQuad.m_pipelineLayout; }
	const VkPipeline DebugPass::getPipeline() const { return m_debugQuad.m_pipeline; }

	const VkDescriptorSet* DebugPass::getDescriptorSet(uint32_t frameNumber) { return &m_shadowmapDescriptorSet[frameNumber % SWAPCHAIN_IMAGE_COUNT]; }
	
	void DebugPass::release(VkDevice device)
	{
		vkDestroySampler(device, m_shadowmapSampler, nullptr);
	}
}