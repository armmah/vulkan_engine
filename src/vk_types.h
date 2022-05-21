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

#include "EngineCore/Scene.h"

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
		
		static bool createCommandBuffers(std::vector<VkCommandBuffer>& commandBufferCollection, uint32_t count, VkCommandPool pool, VkDevice device)
		{
			commandBufferCollection.resize(count);

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = pool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = count;

			return vkAllocateCommandBuffers(device, &allocInfo, commandBufferCollection.data()) == VK_SUCCESS;
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

	struct MemoryBuffer
	{
		static bool createBuffer(VkBuffer& buffer, VkDeviceMemory& memory, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t bufferSize)
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = bufferSize;

			bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
				return false;

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
			auto memTypeIndex = findSuitableProperties(&memProperties, memRequirements.memoryTypeBits, (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

			if (memTypeIndex < 0)
				memTypeIndex = 0; // stupid fallback, don't do this :/
			// Realisitcally we won't hit this case as according to spec, host visible and host coherent is a combination that is guaranteed to be present.

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = memTypeIndex;

			return vkAllocateMemory(device, &allocInfo, nullptr, &memory) == VK_SUCCESS &&
				vkBindBufferMemory(device, buffer, memory, 0) == VK_SUCCESS;
		}

		static int32_t findSuitableProperties(const VkPhysicalDeviceMemoryProperties* pMemoryProperties, uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties)
		{
			const uint32_t memoryCount = pMemoryProperties->memoryTypeCount;
			for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; memoryIndex++)
			{
				const uint32_t memoryTypeBits = (1 << memoryIndex);
				const bool isRequiredMemoryType = memoryTypeBitsRequirement & memoryTypeBits;

				const VkMemoryPropertyFlags properties = pMemoryProperties->memoryTypes[memoryIndex].propertyFlags;
				const bool hasRequiredProperties = (properties & requiredProperties) == requiredProperties;

				if (isRequiredMemoryType && hasRequiredProperties)
					return static_cast<int32_t>(memoryIndex);
			}

			// failed to find memory type
			return -1;
		}

		// Only works for host coherent memory type
		static bool fillMemory(VkDevice device, VkDeviceMemory memory, const void* source, uint32_t size)
		{
			void* data;
			vkMapMemory(device, memory, 0, size, 0, &data);
			memcpy(data, source, static_cast<size_t>(size));
			vkUnmapMemory(device, memory);

			return true;
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

	static void HelloTriangleCommand(VkCommandBuffer buffer, VkPipeline pipeline, VkRenderPass renderPass, VkFramebuffer frameBuffer, VkExtent2D extent, VkBuffer vertexBuffer, uint32_t size)
	{
		auto cbs = CommandBufferScope(buffer);
		{
			auto rps = RenderPassScope(buffer, renderPass, frameBuffer, extent);

			vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			if (vertexBuffer != nullptr)
			{
				VkBuffer vertexBuffers[] = { vertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
			}

			vkCmdDraw(buffer, size, 1, 0, 0);
		}
	}

	static void renderSingleIndexedMesh(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkRenderPass renderPass, VkFramebuffer frameBuffer, VkExtent2D extent, const VkMesh& mesh)
	{
		const VertexAttributes& vAttributes = mesh.vAttributes;
		const IndexAttributes& iAttributes = mesh.iAttributes;

		auto cbs = CommandBufferScope(commandBuffer);
		{
			auto rps = RenderPassScope(commandBuffer, renderPass, frameBuffer, extent);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			vkCmdBindVertexBuffers(commandBuffer, vAttributes.firstBinding, vAttributes.bindingCount, vAttributes.buffers.data(), vAttributes.offsets.data());
			vkCmdBindIndexBuffer(commandBuffer, iAttributes.buffer, iAttributes.offset, iAttributes.indexType);

			vkCmdDrawIndexed(commandBuffer, mesh.iCount, 1, 0, 0, 0);
		}
	}
};