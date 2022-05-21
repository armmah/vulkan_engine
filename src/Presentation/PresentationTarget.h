#pragma once
#include <vk_types.h>
#include "Presentation/HardwareDevice.h"
#include "Presentation/Device.h"
#include <cassert>

namespace Presentation
{
	class PresentationTarget : IRequireInitialization
	{
		bool m_isInitialized = false;

	public:
		VkSurfaceCapabilitiesKHR capabilities;
		VkSwapchainKHR swapchain;

		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		VkRenderPass renderPass;
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;

		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFrameBuffers;

		PresentationTarget(REF<HardwareDevice const> presentationHardware, REF<Device const> presentationDevice, VertexBinding& vBinding)
		{
			m_isInitialized = createPresentationTarget(presentationHardware, presentationDevice, vBinding);
		}

		bool IRequireInitialization::isInitialized() const { return m_isInitialized; }

		void release(VkDevice device)
		{
			vkDestroyPipeline(device, pipeline, nullptr);
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

			vkDestroyRenderPass(device, renderPass, nullptr);

			int count = static_cast<uint32_t>(swapChainImageViews.size());
			assert(swapChainImageViews.size() == swapChainFrameBuffers.size() && "Frame buffer count should be equal to image view count.");
			for (int i = 0; i < count; i++)
			{
				vkDestroyFramebuffer(device, swapChainFrameBuffers[i], nullptr);
				vkDestroyImageView(device, swapChainImageViews[i], nullptr);
			}

			vkDestroySwapchainKHR(device, swapchain, nullptr);
		}

		bool createPresentationTarget(REF<HardwareDevice const> presentationHardware, REF<Device const> presentationDevice, VertexBinding& vBinding)
		{
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(presentationHardware->getActiveGPU(), presentationDevice->getSurface(), &capabilities);
			swapChainExtent = chooseSwapExtent(presentationDevice->getWindowRef());

			auto vkdevice = presentationDevice->getDevice();

			return createSwapChain(3u, presentationHardware, presentationDevice) &&
				createSwapChainImageViews(vkdevice) &&
				createRenderPass(vkdevice) &&
				createFramebuffers(vkdevice) &&

				createGraphicsPipeline(vkdevice, vBinding, VK_CULL_MODE_BACK_BIT);
		}

		bool createGraphicsPipeline(VkDevice device, VertexBinding& vBinding, VkCullModeFlagBits faceCullingMode = VK_CULL_MODE_BACK_BIT)
		{
			return createGraphicsPipeline(device, vBinding, swapChainExtent, faceCullingMode);
		}

		static bool createShaderModule(VkShaderModule& module, const std::vector<char>& code, VkDevice device);

	private:
		VkExtent2D chooseSwapExtent(const SDL_Window* window)
		{
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			{
				return capabilities.currentExtent;
			}
			else
			{
				int width, height;
				SDL_Vulkan_GetDrawableSize(const_cast<SDL_Window*>(window), &width, &height);

				VkExtent2D actualExtent = {
					static_cast<uint32_t>(width),
					static_cast<uint32_t>(height)
				};

				actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				return actualExtent;
			}
		}

		bool createGraphicsPipeline(VkDevice device, VertexBinding& vBinding, VkExtent2D scissorExtent, VkCullModeFlagBits faceCullingMode);

		bool createSwapChain(uint32_t imageCount, REF<HardwareDevice const> swapChainDetails, REF<Device const> device)
		{
			auto surfaceFormat = swapChainDetails->chooseSwapSurfaceFormat();
			auto presentationMode = swapChainDetails->chooseSwapPresentMode();
			auto extent = chooseSwapExtent(device->getWindowRef());

			imageCount = std::max(imageCount, capabilities.minImageCount);
			if (capabilities.maxImageCount != 0)
				imageCount = std::min(imageCount, capabilities.maxImageCount);

			VkSwapchainCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = device->getSurface();
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.preTransform = capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentationMode;
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = VK_NULL_HANDLE;

			bool isSuccess = vkCreateSwapchainKHR(device->getDevice(), &createInfo, nullptr, &swapchain) == VK_SUCCESS;

			if (isSuccess)
			{
				swapChainExtent = extent;
				swapChainImageFormat = surfaceFormat.format;

				vkGetSwapchainImagesKHR(device->getDevice(), swapchain, &imageCount, nullptr);
				swapChainImages.resize(imageCount);
				vkGetSwapchainImagesKHR(device->getDevice(), swapchain, &imageCount, swapChainImages.data());
			}

			return isSuccess;
		}

		bool createRenderPass(VkDevice device)
		{
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = swapChainImageFormat;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			return vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS;
		}

		bool createSwapChainImageViews(VkDevice device)
		{
			auto imageCount = swapChainImages.size();
			swapChainImageViews.resize(imageCount);

			for (int i = 0; i < imageCount; i++)
			{
				VkImageViewCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = swapChainImages[i];
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = swapChainImageFormat;

				createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.baseMipLevel = 0;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.baseArrayLayer = 0;
				createInfo.subresourceRange.layerCount = 1;

				if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
					return false;
			}

			return true;
		}

		bool createFramebuffers(VkDevice device)
		{
			auto imageViewSize = swapChainImageViews.size();
			swapChainFrameBuffers.resize(imageViewSize);

			for (size_t i = 0; i < imageViewSize; i++)
			{
				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderPass;
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = &swapChainImageViews[i];
				framebufferInfo.width = swapChainExtent.width;
				framebufferInfo.height = swapChainExtent.height;
				framebufferInfo.layers = 1;

				if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFrameBuffers[i]) != VK_SUCCESS)
					return false;
			}

			return true;
		}
	};
}