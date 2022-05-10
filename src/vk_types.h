// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "vulkan/vulkan.h"

#pragma warning(push, 0)
#include "SDL.h"
#include "SDL_vulkan.h"
#pragma warning(pop)

#include <vector>
#include <algorithm>
#include <set>
#include <string>
#include <memory>
#include <optional>

#define REF std::shared_ptr
#define MAKEREF std::make_shared

class VulkanValidationLayers
{
#ifdef NDEBUG
	bool m_enableValidationLayers = false;
#else
	bool m_enableValidationLayers = true;
#endif

	const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };

	bool checkValidationLayerSupport();

public:
	VulkanValidationLayers() { }
	VulkanValidationLayers(bool forceValidation) { m_enableValidationLayers = forceValidation; }

	bool checkValidationLayersFailed();

	void applyValidationLayers(VkInstanceCreateInfo& createInfo);
	void applyValidationLayers(VkDeviceCreateInfo& createInfo);
};

class IRequireInitialization
{
public:
	virtual bool isInitialized() const = 0;
};

struct QueueFamilyIndices : IRequireInitialization
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IRequireInitialization::isInitialized() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

namespace vkinit
{

	struct Instance
	{
		static const std::vector<const char*> requiredExtensions;

		static bool getRequiredExtensionsForPlatform(SDL_Window* window, unsigned int* extCount, const char** extensionNames);
		static bool createInstance(VkInstance& instance, std::string applicationName, std::vector<const char*> extNames, std::optional<VulkanValidationLayers> validationLayers);
	};

	struct Surface
	{
		static bool createSurface(VkSurfaceKHR& surface, VkInstance instance, SDL_Window* window)
		{
			return SDL_Vulkan_CreateSurface(window, instance, &surface);
		}
	};

	struct Queue
	{
		static const VkQueueFlags requiredFlags = VK_QUEUE_GRAPHICS_BIT;

		static bool satisfiesQueueRequirements(VkQueueFlags flags);

		static void findQueueFamilies(QueueFamilyIndices& indices, VkPhysicalDevice device, VkSurfaceKHR surface);
		static std::vector<VkDeviceQueueCreateInfo> getQueueCreateInfo(QueueFamilyIndices indices, const float* queuePriority);
		static VkDeviceQueueCreateInfo deviceQueueCreateInfo(uint32_t queueFamily, const float* queuePriority);
	};
	/*
	struct SwapChain
	{
		static HardwarePresentationDevice querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
		static bool createSwapChains(PresentationTarget& swapChainWrapper, uint32_t imageCount, VkDevice device, VkSurfaceKHR surface, HardwarePresentationDevice swapChainDetails, SDL_Window* window);

		static bool createRenderPass(VkRenderPass& renderPass, VkDevice device, VkFormat swapChainImageFormat);
		static bool createSwapChainImageViews(std::vector<VkImageView>& imageViews, const VkDevice device, VkFormat imageFormat, const std::vector<VkImage>& images);
		static bool createFramebuffers(std::vector<VkFramebuffer>& framebufferCollection, VkDevice device, VkRenderPass renderPass, const std::vector<VkImageView>& imageViewCollection, VkExtent2D extent);
	};
	*/
	struct Commands
	{
		static bool createSingleCommandBuffer(VkCommandBuffer& commandBuffer, VkCommandPool pool, VkDevice device)
		{
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = pool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;

			return vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) == VK_SUCCESS;
		}
	};

	struct Synchronization
	{
		static bool createSemaphore(VkSemaphore& semaphore, VkDevice device)
		{
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			return vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) == VK_SUCCESS;
		}
		static bool createFence(VkFence& fence, VkDevice device, bool createSignaled = true)
		{
			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

			if (createSignaled)
			{
				fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			}

			return vkCreateFence(device, &fenceInfo, nullptr, &fence) == VK_SUCCESS;
		}
	};
}

class HardwarePresentationDevice : IRequireInitialization
{
	VkPhysicalDevice m_chosenGPU; // GPU chosen as the default device
	std::vector<VkSurfaceFormatKHR> m_formats;
	std::vector<VkPresentModeKHR> m_presentModes;

	bool m_isInitialized = false;

public:
	HardwarePresentationDevice(VkInstance instance, VkSurfaceKHR surface)
	{
		m_isInitialized = pickPhysicalDevice(instance, surface);
	}

	bool IRequireInitialization::isInitialized() const { return m_isInitialized; }

	VkPhysicalDevice getActiveGPU() const { return m_chosenGPU; }

	VkSurfaceFormatKHR chooseSwapSurfaceFormat() const
	{
		for (const auto& availableFormat : m_formats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return m_formats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode() const
	{
		for (const auto& availablePresentMode : m_presentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

private:
	void querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			m_formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, m_formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			m_presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, m_presentModes.data());
		}
	}

	int rateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		int score = 0;

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		QueueFamilyIndices queueFamily;
		vkinit::Queue::findQueueFamilies(queueFamily, device, surface);
		if (!queueFamily.isInitialized())
		{
			printf("Physical device '%s' is discarded because it doesn't satisfy the QueueManager requirements.\n", deviceProperties.deviceName);
			return 0;
		}

		std::string unsupportedExtensions;
		if (!checkDeviceExtensionSupport(device, vkinit::Instance::requiredExtensions, unsupportedExtensions))
		{
			printf("Physical device '%s' is discarded because it doesn't support the required extensions: %s.\n", deviceProperties.deviceName, unsupportedExtensions.c_str());
			return 0;
		}

		querySwapChainSupport(device, surface);
		if (m_formats.empty() || m_presentModes.empty())
		{
			printf("Physical device '%s' is discarded because the swap chain detail support is not adequete.\n", deviceProperties.deviceName);
			return 0;
		}

		// Discrete GPUs have a significant performance advantage
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		return score;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> requiredExtensions, std::string& unsupportedExtensionNames) const
	{
		uint32_t extCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, availableExtensions.data());

		std::set<std::string> required(requiredExtensions.begin(), requiredExtensions.end());
		for (const auto& ext : availableExtensions)
		{
			required.erase(ext.extensionName);
		}

		if (required.size() > 0)
		{
			unsupportedExtensionNames = "\n{\n";
			for (const auto& name : required)
			{
				unsupportedExtensionNames.append("  '" + name + "'\n");
			}
			unsupportedExtensionNames.append("}");
		}

		return required.empty();
	}

	bool pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
	{
		m_chosenGPU = VK_NULL_HANDLE;

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			printf("failed to find GPUs with Vulkan support!");
			return false;
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		int score = -1;
		VkPhysicalDevice bestDevice;

		for (const auto& device : devices)
		{
			auto newScore = rateDeviceSuitability(device, surface);
			if (newScore > score)
			{
				score = newScore;
				bestDevice = device;
			}
		}

		// Check if the best candidate is suitable at all
		if (score > 0)
		{
			m_chosenGPU = bestDevice;

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(bestDevice, &deviceProperties);
			printf("Chosen the '%s' physical device.\n", deviceProperties.deviceName);
		}
		else
		{
			printf("failed to find a suitable GPU!");
			return false;
		}

		return m_chosenGPU != VK_NULL_HANDLE;
	}
};

class PresentationDevice : IRequireInitialization
{
	VkDevice m_vkdevice;
	VkSurfaceKHR m_surface;
	QueueFamilyIndices m_queueIndices;

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	VkCommandPool m_commandPool;

	const SDL_Window* m_window;
	std::optional<VulkanValidationLayers> m_validationLayers;
	bool m_isInitialized = false;

public:
	PresentationDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const SDL_Window* windowPtr, std::optional<VulkanValidationLayers> validationLayers)
		: m_surface(surface), m_window(windowPtr), m_validationLayers(validationLayers)
	{
		m_isInitialized = 
			createLogicalDevice(physicalDevice) && 
			createCommandPool();
	}

	bool IRequireInitialization::isInitialized() const { return m_isInitialized; }

	VkDevice getDevice() const { return m_vkdevice; }
	VkSurfaceKHR getSurface() const { return m_surface; }
	VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
	VkQueue getPresentQueue() const { return m_presentQueue; }
	VkCommandPool getCommandPool() const { return m_commandPool; }
	const SDL_Window* getWindowRef() const { return m_window; }

	void release()
	{
		vkDestroyCommandPool(m_vkdevice, m_commandPool, nullptr);
		vkDestroyDevice(m_vkdevice, nullptr);
	}

private:
	bool createCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_queueIndices.graphicsFamily.value();

		return vkCreateCommandPool(m_vkdevice, &poolInfo, nullptr, &m_commandPool) == VK_SUCCESS;
	}

	bool createLogicalDevice(VkPhysicalDevice physicalDevice)
	{
		VkPhysicalDeviceFeatures deviceFeatures{};

		vkinit::Queue::findQueueFamilies(m_queueIndices, physicalDevice, m_surface);
		float queuePriority = 1.0;
		auto queueCreateInfos = vkinit::Queue::getQueueCreateInfo(m_queueIndices, &queuePriority);

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		auto& requiredExtensions = vkinit::Instance::requiredExtensions;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		if (m_validationLayers.has_value())
		{
			m_validationLayers.value().applyValidationLayers(createInfo);
		}

		bool isSuccess = vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_vkdevice) == VK_SUCCESS;
		if (isSuccess)
		{
			vkGetDeviceQueue(m_vkdevice, m_queueIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
			vkGetDeviceQueue(m_vkdevice, m_queueIndices.presentFamily.value(), 0, &m_presentQueue);
		}

		return isSuccess;
	}

};

class PresentationTarget : IRequireInitialization
{
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

	bool m_isInitialized = false;

	PresentationTarget(REF<HardwarePresentationDevice const> presentationHardware, REF<PresentationDevice const> presentationDevice)
	{
		m_isInitialized = createPresentationTarget(presentationHardware, presentationDevice);
	}

	bool IRequireInitialization::isInitialized() const { return m_isInitialized; }

	void release(VkDevice device)
	{
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

		vkDestroyRenderPass(device, renderPass, nullptr);

		int count = static_cast<uint32_t>(swapChainImageViews.size());
		//assert(swapChainImageViews.size() == swapChainFrameBuffers.size() && "Frame buffer count should be equal to image view count.");
		for (int i = 0; i < count; i++)
		{
			vkDestroyFramebuffer(device, swapChainFrameBuffers[i], nullptr);
			vkDestroyImageView(device, swapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(device, swapchain, nullptr);
	}

	bool createPresentationTarget(REF<HardwarePresentationDevice const> presentationHardware, REF<PresentationDevice const> presentationDevice)
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(presentationHardware->getActiveGPU(), presentationDevice->getSurface(), &capabilities);
		swapChainExtent = chooseSwapExtent(presentationDevice->getWindowRef());

		auto vkdevice = presentationDevice->getDevice();

		return createSwapChain(3u, presentationHardware, presentationDevice) &&
			createSwapChainImageViews(vkdevice) &&
			createRenderPass(vkdevice) &&
			createFramebuffers(vkdevice) &&

			// todo: refactor this logic under vkinit
			createGraphicsPipeline(vkdevice, VK_CULL_MODE_BACK_BIT);
	}

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

	bool createGraphicsPipeline(VkDevice device, VkCullModeFlagBits faceCullingMode);
	bool createGraphicsPipeline(VkDevice device, VkExtent2D scissorExtent, VkCullModeFlagBits faceCullingMode);

	static bool createShaderModule(VkShaderModule& module, const std::vector<char>& code, VkDevice device);

	bool createSwapChain(uint32_t imageCount, REF<HardwarePresentationDevice const> swapChainDetails, REF<PresentationDevice const> device)
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

#include <fstream>

struct FileIO
{
	static std::vector<char> readFile(const std::string& filename);
};

struct CommandObjectsWrapper
{
	class CommandBufferScope
	{
		VkCommandBuffer commandBuffer;

	public:
		CommandBufferScope(VkCommandBuffer commandBuffer);
		~CommandBufferScope();
	};

	class RenderPassScope
	{
		VkCommandBuffer commandBuffer;

	public:
		RenderPassScope(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer swapChainFramebuffer, VkExtent2D extent);
		~RenderPassScope();
	};

	static void HelloTriangleCommand(VkCommandBuffer buffer, VkPipeline pipeline, VkRenderPass renderPass, VkFramebuffer frameBuffer, VkExtent2D extent);
};

class Frame
{
	VkCommandBuffer buffer;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;

public:
	bool initialize(VkDevice device, VkCommandPool pool)
	{
		bool fullyInitialized = true;
		fullyInitialized &= vkinit::Synchronization::createSemaphore(imageAvailableSemaphore, device);
		fullyInitialized &= vkinit::Synchronization::createSemaphore(renderFinishedSemaphore, device);

		fullyInitialized &= vkinit::Synchronization::createFence(inFlightFence, device);

		fullyInitialized &= vkinit::Commands::createSingleCommandBuffer(buffer, pool, device);
		return fullyInitialized;
	}

	VkCommandBuffer getCommandBuffer() const { return buffer; }
	VkSemaphore getImageAvailableSemaphore() const { return imageAvailableSemaphore; }
	VkSemaphore getRenderFinishedSemaphore() const { return renderFinishedSemaphore; }
	VkFence getInFlightFence() const { return inFlightFence; }

	void submitToQueue(VkQueue graphicsQueue)
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &buffer;

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS)
			throw std::runtime_error("Failed to submit draw command buffer!");
	}

	void present(uint32_t imageIndex, VkSwapchainKHR swapChain, VkQueue presentQueue)
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphore;

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain;
		presentInfo.pImageIndices = &imageIndex;

		presentInfo.pResults = nullptr; // Optional

		vkQueuePresentKHR(presentQueue, &presentInfo);
	}

	void waitOnAcquireFence(VkDevice device)
	{
		vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	}

	void resetAcquireFence(VkDevice device)
	{
		vkResetFences(device, 1, &inFlightFence);
	}

	void release(VkDevice device)
	{
		vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
		vkDestroyFence(device, inFlightFence, nullptr);
	}
};

class PresentationFrames : IRequireInitialization
{
	REF<PresentationDevice const> presentationDevice;
	std::vector<Frame> frameCollection;
	int currentFrameIndex = 0;
	bool fullyInitialized = false;

public:
	PresentationFrames(REF<PresentationDevice const> presentationDevice, int frameCount = 2) :
		presentationDevice(presentationDevice), frameCollection(frameCount)
	{
		fullyInitialized = true;

		auto device = presentationDevice->getDevice();
		auto pool = presentationDevice->getCommandPool();
		for (int i = 0; i < frameCount; i++)
		{
			fullyInitialized &= frameCollection[i].initialize(device, pool);
		}
	}

	bool IRequireInitialization::isInitialized() const { return fullyInitialized; }

	Frame getNextFrame()
	{
		currentFrameIndex = (currentFrameIndex + 1) % frameCollection.size();
		return frameCollection[currentFrameIndex];
	}

	Frame getNextFrameAndWaitOnFence()
	{
		auto frame = getNextFrame();
		frame.waitOnAcquireFence(presentationDevice->getDevice());
		return frame;
	}

	VkResult acquireImageFromSwapchain(uint32_t& imageIndex, VkSwapchainKHR swapchain)
	{
		return vkAcquireNextImageKHR(presentationDevice->getDevice(), swapchain, UINT64_MAX, frameCollection[currentFrameIndex].getImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);
	}

	void releaseFrameResources()
	{
		for (auto& frame : frameCollection)
		{
			frame.release(presentationDevice->getDevice());
		}

		frameCollection.clear();
		currentFrameIndex = 0;
		fullyInitialized = false;
	}
};
