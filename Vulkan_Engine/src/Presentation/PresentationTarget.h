#pragma once
#include "pch.h"
#include "Common.h"
#include "Interfaces/IRequireInitialization.h"
//#include "VkTypes/VkMaterialVariant.h"

struct VkGraphicsPipeline
{
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

class Window;
struct VkTexture;
struct VkTexture2D;
struct Shader;
class Material;
struct VertexBinding;

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

		VkImage getSwapchainImage(uint32_t index) const { return m_swapChainImages[index]; }
		VkImageView getSwapchainImageView(uint32_t index) const { return m_swapChainImageViews[index]; }
		VkFramebuffer getSwapchainFrameBuffers(uint32_t index) const { return m_swapChainFrameBuffers[index]; }
		bool hasDepthAttachement();

		bool createPresentationTarget(const HardwareDevice& presentationHardware, const Device& presentationDevice, uint32_t swapchainCount = 3u);
		bool createMaterial(UNQ<Material>& material, VkDevice device, VkDescriptorPool descPool, const Shader* shader, const VkTexture2D* texture);

		void release(VkDevice device);

		std::unordered_map<const Shader*, VkGraphicsPipeline> globalPipelineList;
		std::unordered_map<const Shader*, VkDescriptorSetLayout> globalDescriptorSetLayoutList;
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
		bool createGraphicsPipeline(VkPipeline& pipeline, VkPipelineLayout& layout, const Shader& shader, VkDevice device, const VertexBinding& vBinding, VkDescriptorSetLayout descriptorSetLayout, VkCullModeFlagBits faceCullingMode = VK_CULL_MODE_BACK_BIT, bool depthStencilAttachement = true) const;
	};
}