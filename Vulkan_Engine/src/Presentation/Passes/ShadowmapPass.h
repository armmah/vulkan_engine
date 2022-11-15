#pragma once
#include "pch.h"
#include "Presentation/Passes/Pass.h"
#include "Interfaces/IRequireInitialization.h"
#include "VkTypes/VkTexture.h"
#include "VkGraphicsPipeline.h"

struct VkShader;

namespace Presentation
{
	class ShadowMap : public Pass, IRequireInitialization
	{
		static constexpr uint32_t MIN_SHADOWMAP_DIMENSION = 64u;
		static constexpr uint32_t MAX_SHADOWMAP_DIMENSION = 2048u;

		static constexpr VkFormat FORMAT = VK_FORMAT_D32_SFLOAT;
		static constexpr VkImageUsageFlagBits USAGE_FLAGS = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		static constexpr VkImageAspectFlagBits VIEW_IMAGE_ASPECT_FLAGS = VK_IMAGE_ASPECT_DEPTH_BIT;

	public:
		ShadowMap(VkDevice device, VkPipelineLayout depthOnlyPipelineLayout, bool isEnabled, uint32_t dimensionsXY);

		bool isInitialized() const override;
		const VkViewport& getViewport() const;
		const VkRect2D& getScissorRect() const;
		const VkExtent2D getExtent() const;
		const VkRenderPass getRenderPass() const;
		const VkFramebuffer getFrameBuffer(uint32_t frameNumber) const;
		const VkTexture2D& getTexture2D() const;
		VkFormat getFormat() const;

		virtual void release(VkDevice device) override;

		const VkShader* m_replacementShader;
		VkGraphicsPipeline m_replacementMaterial;

	private:
		uint32_t m_dimensionsXY;
		VkTexture2D m_shadowMap;

		VkRenderPass m_renderPass;
		std::array<VkFramebuffer, SWAPCHAIN_IMAGE_COUNT> m_frameBuffer;

		VkViewport m_viewport;
		VkRect2D m_scissorRect;
		VkExtent2D m_extent;

		bool m_isInitialized;
	};
}