#include "pch.h"
#include "DebugPass.h"
#include "VkTypes/PipelineConstructor.h"
#include "DescriptorPoolManager.h"
#include "Material.h"
// #include "VkTypes/VkMaterialVariant.h"
#include "PresentationTarget.h"

namespace Presentation
{
	DebugPass::DebugPass() : Pass(false), m_debugQuad() { }

	DebugPass::~DebugPass() { }

	DebugPass::DebugPass(PresentationTarget& target, VkDevice device, const VkShader* shader, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, VkExtent2D extent, const VkTexture2D& displayTexture)
		: Pass(true)
	{
		m_debugQuad.m_pipelineLayout = pipelineLayout;
		if (PipelineConstruction::createPipeline(m_debugQuad.m_pipeline, pipelineLayout, device,
			renderPass, extent, *shader, nullptr, PipelineConstruction::FaceCulling::None, true))
		{
			auto pool = DescriptorPoolManager::getInstance()->createNewPool(3u);
			target.createGraphicsMaterial(m_debugMaterial, device, pool, shader, &displayTexture);
		}
		else printf("Could not create pipeline for the debug quad shader.\n");
	}

	const VkGraphicsPipeline& DebugPass::getGraphicsPipeline() const { return m_debugQuad; }

	const VkPipeline DebugPass::getPipeline() const { return m_debugQuad.m_pipeline; }

	const VkMaterialVariant& DebugPass::getMaterialVariant() const { return m_debugMaterial->getMaterialVariant(); }

	void DebugPass::release(VkDevice device)
	{
		if (m_debugMaterial)
		{
			m_debugMaterial->release(device);
			m_debugMaterial = nullptr;
		}
	}
}