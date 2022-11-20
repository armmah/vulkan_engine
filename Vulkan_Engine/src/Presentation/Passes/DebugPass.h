#pragma once
#include "pch.h"
#include "Common.h"
#include "Interfaces/IRequireInitialization.h"
#include "VkGraphicsPipeline.h"
#include "Presentation/Passes/Pass.h"

struct VkShader;
struct VkTexture2D;
struct VkMaterialVariant;

namespace Presentation
{
	class PresentationTarget;

	class DebugPass : public Presentation::Pass, IRequireInitialization
	{
	public:
		DebugPass();
		~DebugPass();
		DebugPass(PresentationTarget& target, VkDevice device, const VkShader* shader, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, VkExtent2D extent, const VkTexture2D& displayTexture);

		virtual bool isInitialized() const override;

		const VkPipelineLayout getPipelineLayout() const;
		const VkPipeline getPipeline() const;
		const VkDescriptorSet* getDescriptorSet(uint32_t frameNumber);

		void release(VkDevice device) override;

	private:
		bool m_isInitialized;
		VkGraphicsPipeline m_debugQuad;

		VkSampler m_shadowmapSampler;
		std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT> m_shadowmapDescriptorSet;
	};
}