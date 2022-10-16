#pragma once
#include "pch.h"
#include "Common.h"
#include "Interfaces/IRequireInitialization.h"

#include "Engine/RenderLoopStatistics.h"
#include "VkTypes/InitializersUtility.h"

class Window;
struct VkTexture;
struct VkTexture2D;
struct VkShader;
struct VkMaterial;
struct VertexBinding;
struct VkMeshRenderer;

class Camera;
struct BuffersUBO;
struct BufferHandle;
struct PipelineDescriptor;

namespace Presentation
{
	class Device;
	class HardwareDevice;

	class PresentationTarget : IRequireInitialization
	{
	public:
		PresentationTarget(const HardwareDevice& presentationHardware, const Device& presentationDevice, Window const* wnd, bool depthAttachment, uint32_t swapchainCount = SWAPCHAIN_IMAGE_COUNT);
		~PresentationTarget();

		bool IRequireInitialization::isInitialized() const override { return m_isInitialized; }

		VkSwapchainKHR getSwapchain() const { return m_swapchain; }
		VkExtent2D getSwapchainExtent() const { return m_swapChainExtent; }
		VkRenderPass getRenderPass() const { return m_renderPass; }

		VkImage getSwapchainImage(uint32_t index) const { return m_swapChainImages[index % SWAPCHAIN_IMAGE_COUNT]; }
		VkImageView getSwapchainImageView(uint32_t index) const { return m_swapChainImageViews[index % SWAPCHAIN_IMAGE_COUNT]; }
		VkFramebuffer getSwapchainFrameBuffers(uint32_t index) const { return m_swapChainFrameBuffers[index % SWAPCHAIN_IMAGE_COUNT]; }
		bool hasDepthAttachement();

		bool createPresentationTarget(const HardwareDevice& presentationHardware, const Device& presentationDevice, uint32_t swapchainCount = 3u);
		bool createGraphicsMaterial(UNQ<VkMaterial>& material, VkDevice device, VkDescriptorPool descPool, const VkShader* shader, const VkTexture2D* texture);

		FrameStats renderLoop(const std::vector<VkMeshRenderer>& renderers, Camera& cam, VkCommandBuffer commandBuffer, uint32_t frameNumber);

		void releaseAllResources(VkDevice device);
		void releaseSwapChain(VkDevice device);

		UNQ<PipelineDescriptor> m_globalPipelineState;
	private:
		bool m_isInitialized = false;
		bool m_hasDepthAttachment = false;

		VkSurfaceCapabilitiesKHR m_capabilities;
		VkSwapchainKHR m_swapchain;
		UNQ<VkTexture> m_depthImage;

		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;

		VkRenderPass m_renderPass;

		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews;
		std::vector<VkFramebuffer> m_swapChainFrameBuffers;


		const Window* m_window;

		bool createSwapChain(uint32_t imageCount, const HardwareDevice& hardware, const Device& device, bool createDepthAttachement = true);
		bool createRenderPass(VkDevice device);
		bool createSwapChainImageViews(VkDevice device);
		bool createFramebuffers(VkDevice device);

		VkExtent2D chooseSwapExtent(const SDL_Window* window);
		bool createGraphicsPipeline(VkPipeline& pipeline, const VkPipelineLayout layout, const VkShader& shader, VkDevice device, const VertexBinding& vBinding, VkCullModeFlagBits faceCullingMode = VK_CULL_MODE_BACK_BIT, bool depthStencilAttachement = true) const;

		void renderIndexedMeshes(FrameStats& stats, const std::vector<VkMeshRenderer>& renderers, const Camera& cam, VkCommandBuffer commandBuffer, uint32_t frameNumber);
	};
}