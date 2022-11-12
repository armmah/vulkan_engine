#pragma once
#include "pch.h"
#include "Common.h"
#include "Interfaces/IRequireInitialization.h"

#include "Engine/RenderLoopStatistics.h"
#include "VkTypes/InitializersUtility.h"
#include "PipelineBinding.h"
#include "Material.h"

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

#include "VkTypes/VkTexture.h"

namespace Presentation
{
	class Pass
	{
	public:
		Pass(bool isEnabled) : m_isEnabled(isEnabled) { }

		virtual void release(VkDevice device) = 0;

	private:
		bool m_isEnabled;
	};

	class ShadowMap : Pass, IRequireInitialization
	{
		static constexpr uint32_t MIN_SHADOWMAP_DIMENSION = 64u;
		static constexpr uint32_t MAX_SHADOWMAP_DIMENSION = 2048u;

		static constexpr VkFormat FORMAT = VK_FORMAT_D32_SFLOAT;
		static constexpr VkImageUsageFlagBits USAGE_FLAGS = static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		static constexpr VkImageAspectFlagBits VIEW_IMAGE_ASPECT_FLAGS = VK_IMAGE_ASPECT_DEPTH_BIT;

	public:
		ShadowMap(VkDevice device, bool isEnabled, uint32_t dimensionsXY) : Pass(isEnabled),
			m_dimensionsXY(std::clamp(dimensionsXY, MIN_SHADOWMAP_DIMENSION, MAX_SHADOWMAP_DIMENSION)),
			m_renderPass(), m_frameBuffer(), m_isInitialized(false),
			m_shadowMap(VkTexture2D::createTexture(device, m_dimensionsXY, m_dimensionsXY, FORMAT, USAGE_FLAGS, VIEW_IMAGE_ASPECT_FLAGS)),
			m_replacementShader(), m_replacementMaterial()
		{
			m_extent = {};
			m_extent.width = dimensionsXY;
			m_extent.height = dimensionsXY;

			vkinit::Commands::initViewportAndScissor(m_viewport, m_scissorRect, m_extent);

			std::array<VkImageView, 1> imageViews {
				m_shadowMap.imageView
			};

			m_isInitialized =
				m_shadowMap.isValid() &&
				vkinit::Surface::createRenderPass(m_renderPass, device, (VkFormat)0, true);

			for (auto& fb : m_frameBuffer)
			{
				m_isInitialized &= vkinit::Surface::createFrameBuffer(fb, device, m_renderPass, m_extent, imageViews);
			}
		}

		bool isInitialized() const override { return m_isInitialized; }
		const VkViewport& getViewport() const { return m_viewport; }
		const VkRect2D& getScissorRect() const { return m_scissorRect; }
		const VkExtent2D getExtent() const { return m_extent; }
		const VkRenderPass getRenderPass() const { return m_renderPass; }
		const VkFramebuffer getFrameBuffer(uint32_t frameNumber) const { return m_frameBuffer[frameNumber % SWAPCHAIN_IMAGE_COUNT]; }
		const VkTexture2D& getTexture2D() const { return m_shadowMap; }
		VkFormat getFormat() const { return FORMAT; }

		virtual void release(VkDevice device) override
		{
			m_shadowMap.release(device);

			for (auto& fb : m_frameBuffer)
			{
				vkDestroyFramebuffer(device, fb, nullptr);
			}
			vkDestroyRenderPass(device, m_renderPass, nullptr);
		}

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
		bool createPipelineIfNotExist(VkGraphicsPipeline& graphicsPipeline, const VkPipelineLayout pipelineLayout,
			const VkDevice device, const VkShader* shader, const VkRenderPass renderPass, VkExtent2D extent);
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
		VkViewport m_viewport;
		VkRect2D m_scissorRect;

		VkRenderPass m_renderPass;
		UNQ<ShadowMap> m_shadowMapModule;
		VkGraphicsPipeline m_debugQuad;
		UNQ<VkMaterial> m_debugMaterial;

		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews;
		std::vector<VkFramebuffer> m_swapChainFrameBuffers;

		const Window* m_window;

		bool createSwapChain(uint32_t imageCount, const HardwareDevice& hardware, const Device& device, bool createDepthAttachement = true);
		bool createRenderPass(VkDevice device);
		bool createSwapChainImageViews(VkDevice device);
		bool createFramebuffers(VkDevice device);

		VkExtent2D chooseSwapExtent(const SDL_Window* window);

		void renderIndexedMeshes(FrameStats& stats, const std::vector<VkMeshRenderer>& renderers, Camera& cam, VkCommandBuffer commandBuffer, uint32_t frameNumber);
	};
}