#pragma once
#include "pch.h"
#include "Common.h"
#include "VkGraphicsPipeline.h"
#include "Presentation/Passes/Pass.h"

struct VkShader;
struct VkTexture2D;
struct VkMaterialVariant;
struct VkMaterial;

namespace Presentation
{
	class PresentationTarget;

	class DebugPass : public Presentation::Pass
	{
	public:
		DebugPass();
		~DebugPass();
		DebugPass(PresentationTarget& target, VkDevice device, const VkShader* shader, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, VkExtent2D extent, const VkTexture2D& displayTexture);

		const VkGraphicsPipeline& getGraphicsPipeline() const;
		const VkPipeline getPipeline() const;
		const VkMaterialVariant& getMaterialVariant() const;

		void release(VkDevice device) override;

	private:
		VkGraphicsPipeline m_debugQuad;
		UNQ<VkMaterial> m_debugMaterial;
	};
}