﻿// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "vulkan/vulkan.h"

#include <codeanalysis\warnings.h>
#pragma warning(push, 0)
#pragma warning ( disable : ALL_CPPCORECHECK_WARNINGS )
#include "SDL.h"
#include "SDL_vulkan.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_vulkan.h"
#pragma warning(pop)

#include "EngineCore/pch.h"

#include "EngineCore/Scene.h"

class VulkanValidationLayers
{
	const char* m_validationLayerName = "VK_LAYER_KHRONOS_validation";

	bool checkValidationLayerSupport();

public:
	VulkanValidationLayers() { }

	bool checkValidationLayersFailed();

	void applyValidationLayers(VkInstanceCreateInfo& createInfo) const;
	void applyValidationLayers(VkDeviceCreateInfo& createInfo) const;
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
		static bool createInstance(VkInstance& instance, std::string applicationName, std::vector<const char*> extNames, const VulkanValidationLayers* validationLayers);
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

		static void initViewportAndScissor(VkViewport& viewport, VkRect2D& scissor, VkExtent2D extent)
		{
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(extent.width);
			viewport.height = static_cast<float>(extent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			scissor.offset = { 0, 0 };
			scissor.extent = extent;
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
		static bool createVmaAllocator(VmaAllocator& vmaAllocator, const VkInstance instance, const VkPhysicalDevice physicalDevice, const VkDevice device)
		{
			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.instance = instance;
			allocatorInfo.physicalDevice = physicalDevice;
			allocatorInfo.device = device;

			return vmaCreateAllocator(&allocatorInfo, &vmaAllocator) == VK_SUCCESS;
		}
		
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

struct ShaderSource
{
	std::string vertexPath,
		fragmentPath;

	ShaderSource(std::string path_vertexShaderSource, std::string path_fragmentShaderSource)
		: vertexPath(path_vertexShaderSource), fragmentPath(path_fragmentShaderSource) { }

	std::vector<char> getVertexSource() { return FileIO::readFile(vertexPath); }
	std::vector<char> getFragmentSource() { return FileIO::readFile(fragmentPath); }

	static ShaderSource getHardcodedTriangle() { return ShaderSource("C:/Git/Vulkan_Engine/Shaders/outputSPV/triangle.vert.spv", "C:/Git/Vulkan_Engine/Shaders/outputSPV/triangle.frag.spv"); }
	static ShaderSource getDefaultShader() { return ShaderSource("C:/Git/Vulkan_Engine/Shaders/outputSPV/simple.vert.spv", "C:/Git/Vulkan_Engine/Shaders/outputSPV/simple.frag.spv"); }
};

struct Shader
{
	VkDevice device;
	VkShaderModule vertShader,
		fragShader;

	Shader(VkDevice device, ShaderSource source)
		: device(device)
	{
		if (!createShaderModule(vertShader, source.getVertexSource(), device))
		{
			printf("Failed to compile the shader '%s'.", source.fragmentPath.c_str());
		}

		if (!createShaderModule(fragShader, source.getFragmentSource(), device))
		{
			printf("Failed to compile the shader '%s'.", source.fragmentPath.c_str());
		}
	}

	static bool createShaderModule(VkShaderModule& module, const std::vector<char>& code, VkDevice device)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		return vkCreateShaderModule(device, &createInfo, nullptr, &module) == VK_SUCCESS;
	}

	void release()
	{
		vkDestroyShaderModule(device, vertShader, nullptr);
		vkDestroyShaderModule(device, fragShader, nullptr);
	}
};

class Camera
{
public:
	Camera(float fov_degrees, VkExtent2D windowSize, float nearZ = 1.0f, float farZ = 100.0f) :
		fov_radians(glm::radians(fov_degrees)),
		windowExtent(windowSize),
		nearZ(nearZ), farZ(farZ),
		pos(0.f), rot(glm::vec3(0.f, 0.f, 0.f))
	{
		calculateViewMatrix();
		updateWindowExtent(windowExtent);
	}

	const glm::vec3& getPosition() const { return pos; }
	const glm::quat& getRotation() const { return rot; }

	void setPosition(const glm::vec3& position) { pos = position; calculateViewMatrix(); }
	void setRotation(const glm::quat& rotation) { rot = rotation; calculateViewMatrix(); }
	void setRotation(const glm::vec3& rotEuler) { rot = glm::quat(rotEuler); calculateViewMatrix(); }

	const VkViewport& getViewport() const { return viewport; }
	const VkRect2D& getScissorRect() const { return scissor; }

	void setNearFarZ(float near, float far)
	{
		nearZ = near;
		farZ = far;
	}

	void setFieldOfView(float fov_degrees)
	{
		auto newFoV = glm::radians(fov_degrees);
		if (newFoV == fov_radians)
			return;

		fov_radians = newFoV;
		calculateProjectionMatrix();
	}

	void updateWindowExtent(VkExtent2D newExtent)
	{
		if (newExtent.width == windowExtent.width &&
			newExtent.height == newExtent.width)
			return;

		windowExtent = newExtent;

		aspectRatio = windowExtent.width / (float)windowExtent.height;
		calculateProjectionMatrix();

		vkinit::Commands::initViewportAndScissor(viewport, scissor, newExtent);
	}

	const glm::mat4& getViewProjectionMatrix() const { return cachedViewProjectionMatrix; }

private:
	float fov_radians;
	float aspectRatio;
	float nearZ;
	float farZ;

	glm::vec3 pos;
	glm::quat rot;

	VkExtent2D windowExtent;
	VkViewport viewport;
	VkRect2D scissor;

	glm::mat4 cachedViewMatrix;
	glm::mat4 cachedProjectionMatrix;
	glm::mat4 cachedViewProjectionMatrix;

	glm::mat4 calculateViewMatrix()
	{
		glm::mat4 rotationMatrix = glm::mat4_cast(rot);
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.f), pos);

		cachedViewMatrix = translationMatrix * rotationMatrix;
		cachedViewProjectionMatrix = calculateViewProjectionMatrix();
		return cachedViewMatrix;
	}

	glm::mat4 calculateProjectionMatrix()
	{
		cachedProjectionMatrix = glm::perspective(fov_radians, aspectRatio, nearZ, farZ);
		cachedViewProjectionMatrix = calculateViewProjectionMatrix();
		return cachedProjectionMatrix;
	}

	glm::mat4 calculateViewProjectionMatrix() { return cachedProjectionMatrix * cachedViewMatrix; }
};

struct TransformPushConstant
{
	glm::mat4 mvp_matrix;
};

struct CommandObjectsWrapper
{
	class CommandBufferScope
	{
		VkCommandBuffer commandBuffer;

	public:
		CommandBufferScope(VkCommandBuffer commandBuffer);
		~CommandBufferScope();

		CommandBufferScope(const CommandBufferScope&) = delete;
		CommandBufferScope& operator=(const CommandBufferScope&) = delete;
	};

	class RenderPassScope
	{
		VkCommandBuffer commandBuffer;

	public:
		RenderPassScope(VkCommandBuffer commandBuffer, VkRenderPass m_renderPass, VkFramebuffer swapChainFramebuffer, VkExtent2D extent);
		~RenderPassScope();

		RenderPassScope(const RenderPassScope&) = delete;
		RenderPassScope& operator=(const RenderPassScope&) = delete;
	};

	static void HelloTriangleCommand(VkCommandBuffer buffer, VkPipeline m_pipeline, VkRenderPass m_renderPass, VkFramebuffer frameBuffer, VkExtent2D extent, VkBuffer vertexBuffer, uint32_t size)
	{
		auto cbs = CommandBufferScope(buffer);
		{
			auto rps = RenderPassScope(buffer, m_renderPass, frameBuffer, extent);

			vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

			if (vertexBuffer != nullptr)
			{
				VkBuffer vertexBuffers[] = { vertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
			}

			vkCmdDraw(buffer, size, 1, 0, 0);
		}
	}

	static void drawAt(VkCommandBuffer commandBuffer, const VkMesh& mesh, VkPipelineLayout layout,
		const Camera& cam, uint32_t frameNumber, float freq, glm::vec3 pos)
	{
		glm::mat4 model =
			glm::translate(glm::mat4(1.f), pos) *
			glm::rotate(glm::mat4{ 1.0f }, glm::radians(frameNumber * 0.01f * freq), glm::vec3(0, 1, 0));

		TransformPushConstant pushConstant{};
		pushConstant.mvp_matrix = cam.getViewProjectionMatrix() * model;

		mesh.vAttributes->bind(commandBuffer);
		mesh.iAttributes->bind(commandBuffer);

		vkCmdPushConstants(commandBuffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(TransformPushConstant), &pushConstant);
		vkCmdDrawIndexed(commandBuffer, mesh.iCount, 1, 0, 0, 0);
	}

	static void renderIndexedMeshes(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, VkRenderPass m_renderPass, 
		VkFramebuffer frameBuffer, VkExtent2D extent, Camera& cam, const std::vector<UNQ<VkMesh>>& meshes, uint32_t frameNumber)
	{
		auto cbs = CommandBufferScope(commandBuffer);
		{
			cam.updateWindowExtent(extent);

			vkCmdSetViewport(commandBuffer, 0, 1, &cam.getViewport());
			vkCmdSetScissor(commandBuffer, 0, 1, &cam.getScissorRect());

			auto rps = RenderPassScope(commandBuffer, m_renderPass, frameBuffer, extent);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			for (int i = 0; i < meshes.size(); i++)
			{
				auto sign = (i * 2 - 1);
				drawAt(commandBuffer, *meshes[i], pipelineLayout, cam, frameNumber, sign * (i * 10.0f + 0.2f), glm::vec3(sign * 0.2f , sign * 0.2f, 0.0f));
			}

			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
		}
	}
};