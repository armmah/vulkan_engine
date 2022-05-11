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