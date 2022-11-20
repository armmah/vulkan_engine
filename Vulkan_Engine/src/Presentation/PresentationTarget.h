#pragma once
#include "pch.h"
#include "Common.h"
#include "VkGraphicsPipeline.h"
#include "Interfaces/IRequireInitialization.h"

#include "Engine/RenderLoopStatistics.h"

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

	class ShadowMap;
	class DebugPass;

	class PresentationTarget : IRequireInitialization
	{
	public:
		PresentationTarget(const HardwareDevice& presentationHardware, const Device& presentationDevice, Window const* wnd, bool depthAttachment, uint32_t swapchainCount = SWAPCHAIN_IMAGE_COUNT);
		~PresentationTarget();

		bool isInitialized() const override;

		VkSwapchainKHR getSwapchain() const;
		VkExtent2D getSwapchainExtent() const;
		VkRenderPass getRenderPass() const;

		VkImage getSwapchainImage(uint32_t index) const;
		VkImageView getSwapchainImageView(uint32_t index) const;
		VkFramebuffer getSwapchainFrameBuffers(uint32_t index) const;
		bool hasDepthAttachement();

		bool createPresentationTarget(const HardwareDevice& presentationHardware, const Device& presentationDevice, uint32_t swapchainCount = 3u);
		bool createGraphicsMaterial(UNQ<VkMaterial>& material, VkDevice device, VkDescriptorPool descPool, const VkShader* shader, const VkTexture2D* texture);

		FrameStats renderLoop(const std::vector<VkMeshRenderer>& renderers, Camera& cam, DirectionalLightParams& lightTr, VkCommandBuffer commandBuffer, uint32_t frameNumber);
		void applyFrameConfiguration(const FrameSettings* settings);

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
		VkViewport m_viewport;
		VkRect2D m_scissorRect;

		VkRenderPass m_renderPass;
		UNQ<ShadowMap> m_shadowMapModule;
		UNQ<DebugPass> m_debugModule;

		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews;
		std::vector<VkFramebuffer> m_swapChainFrameBuffers;

		const Window* m_window;

		bool createPipelineIfNotExist(VkGraphicsPipeline& graphicsPipeline, const VkPipelineLayout pipelineLayout,
			const VkDevice device, const VkShader* shader, const VkRenderPass renderPass, VkExtent2D extent);
		bool createSwapChain(uint32_t imageCount, const HardwareDevice& hardware, const Device& device, bool createDepthAttachement = true);
		bool createRenderPass(VkDevice device);
		bool createSwapChainImageViews(VkDevice device);
		bool createFramebuffers(VkDevice device);

		VkExtent2D chooseSwapExtent(const SDL_Window* window);

		void renderIndexedMeshes(FrameStats& stats, const std::vector<VkMeshRenderer>& renderers, Camera& cam,
			const BufferHandle& lightViewUBO, VkCommandBuffer commandBuffer, uint32_t frameNumber);
	};
}