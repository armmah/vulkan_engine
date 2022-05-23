#pragma once
#include <vk_types.h>
#include "Presentation/HardwareDevice.h"
#include "Presentation/Device.h"
#include <cassert>

namespace Presentation
{
	class PresentationTarget : IRequireInitialization
	{
	public:
		PresentationTarget(const HardwareDevice& presentationHardware, const Device& presentationDevice, uint32_t swapchainCount = 3u)
		{
			m_isInitialized = createPresentationTarget(presentationHardware, presentationDevice, swapchainCount);
		}

		bool IRequireInitialization::isInitialized() const { return m_isInitialized; }

		VkSwapchainKHR getSwapchain() const { return m_swapchain; }
		VkExtent2D getSwapchainExtent() const { return m_swapChainExtent; }
		VkRenderPass getRenderPass() const { return m_renderPass; }
		VkPipeline getPipeline() const { return m_pipeline; }
		VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }

		VkImage getSwapchainImage(uint32_t index) const { return m_swapChainImages[index]; }
		VkImageView getSwapchainImageView(uint32_t index) const { return m_swapChainImageViews[index]; }
		VkFramebuffer getSwapchainFrameBuffers(uint32_t index) const { return m_swapChainFrameBuffers[index]; }

		bool createPresentationTarget(const HardwareDevice& presentationHardware, const Device& presentationDevice, uint32_t swapchainCount = 3u);
		bool createGraphicsPipeline(VkDevice device, const VertexBinding& vBinding, VkCullModeFlagBits faceCullingMode = VK_CULL_MODE_BACK_BIT);

		void releasePipeline(VkDevice device);
		void release(VkDevice device);

	private:
		bool m_isInitialized = false;

		VkSurfaceCapabilitiesKHR m_capabilities;
		VkSwapchainKHR m_swapchain;

		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;

		VkRenderPass m_renderPass;
		VkPipeline m_pipeline;
		VkPipelineLayout m_pipelineLayout;

		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews;
		std::vector<VkFramebuffer> m_swapChainFrameBuffers;

		bool createSwapChain(uint32_t imageCount, const HardwareDevice& swapChainDetails, const Device& device);
		bool createRenderPass(VkDevice device);
		bool createSwapChainImageViews(VkDevice device);
		bool createFramebuffers(VkDevice device);

		VkExtent2D chooseSwapExtent(const SDL_Window* window);
	};
}