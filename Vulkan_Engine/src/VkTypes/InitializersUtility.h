#pragma once
#include "pch.h"
#include "Common.h"
#include "VkMemoryAllocator.h"

class Window;
struct VkTexture2D;
class VulkanValidationLayers;
struct BuffersUBO;

namespace vkinit
{

	struct Instance
	{
		static const std::vector<const char*> requiredExtensions;

		static bool getRequiredExtensionsForPlatform(Window const* window, unsigned int* extCount, const char** extensionNames);
		static bool createInstance(VkInstance& instance, const char* applicationName, std::vector<const char*> extNames, const VulkanValidationLayers* validationLayers);
		static bool createInstance(VkInstance& instance, const Window* window, const VulkanValidationLayers* validationLayers);
	};

	struct Surface
	{
		static bool createSurface(VkSurfaceKHR& surface, VkInstance instance, const Window* window);
		static bool createRenderPass(VkRenderPass& renderPass, VkDevice device, VkFormat swapchainImageFormat, bool enableDepthAttachment);

		template<size_t _Size>
		static bool createFrameBuffer(VkFramebuffer& frameBuffer, VkDevice device, VkRenderPass renderPass, VkExtent2D extent, std::array<VkImageView, _Size> imageViews, uint32_t count = std::numeric_limits<uint32_t>::max())
		{
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = std::min(as_uint32(imageViews.size()), count);
			framebufferInfo.pAttachments = imageViews.data();
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;

			return vkCreateFramebuffer(device, &framebufferInfo, nullptr, &frameBuffer) == VK_SUCCESS;
		}
	};

	struct Commands
	{
		static bool createSingleCommandBuffer(VkCommandBuffer& commandBuffer, VkCommandPool pool, VkDevice device);
		static bool createCommandBuffers(std::vector<VkCommandBuffer>& commandBufferCollection, uint32_t count, VkCommandPool pool, VkDevice device);
		static void initViewportAndScissor(VkViewport& viewport, VkRect2D& scissor, VkExtent2D extent, int32_t offsetX = 0, int32_t offsetY = 0);
	};

	struct Synchronization
	{
		static bool createSemaphore(VkSemaphore& semaphore, VkDevice device);
		static bool createFence(VkFence& fence, VkDevice device, bool createSignaled = true);
	};

	struct Texture
	{
		static bool createImage(VkImage& image, VmaAllocation& memoryRange, const MemAllocationInfo& allocInfo, VkFormat format, VkImageUsageFlags usageFlags, uint32_t width, uint32_t height, uint32_t mipCount);
		static bool createTextureImageView(VkImageView& imageView, VkDevice device, VkImage image, VkFormat format, uint32_t mipCount, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
		static bool createTextureSampler(VkSampler& sampler, VkDevice device, uint32_t mipCount, bool linearFiltering = true, VkSamplerAddressMode sampleMode = VK_SAMPLER_ADDRESS_MODE_REPEAT, float anisotropySamples = 0.0f);
		static bool createSwapchain(VkSwapchainKHR& swapchain, VkDevice device, VkSurfaceKHR surface, uint32_t imageCount, VkExtent2D extent,
			VkPresentModeKHR presentationMode, VkSurfaceFormatKHR surfaceFormat, VkSurfaceTransformFlagBitsKHR currentTransform);
	};

	struct ShaderBindingArgs
	{
		VkDescriptorType type;
		VkShaderStageFlags shaderStages;

		ShaderBindingArgs(VkDescriptorType type, VkShaderStageFlags shaderStages) : type(type), shaderStages(shaderStages) { }
	};

	struct ShaderBinding
	{
		VkDescriptorSetLayoutBinding getLayoutBinding() const
		{
			VkDescriptorSetLayoutBinding setLayoutBinding{};
			setLayoutBinding.binding = 0;
			setLayoutBinding.descriptorCount = m_descCount;
			setLayoutBinding.descriptorType = m_type;
			setLayoutBinding.pImmutableSamplers = nullptr;
			setLayoutBinding.stageFlags = m_shaderStages;

			return setLayoutBinding;
		}

		ShaderBinding(VkDescriptorType type, VkShaderStageFlags shaderStages) : m_type(type), m_shaderStages(shaderStages), m_descCount(1) { }
	private:
		VkDescriptorType m_type;
		VkShaderStageFlags m_shaderStages;
		uint32_t m_descCount;
	};

	struct BoundBuffer : ShaderBinding
	{
		BoundBuffer(VkShaderStageFlags stageFlags) : ShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stageFlags) { }
	};

	struct BoundTexture : ShaderBinding
	{
		BoundTexture(VkShaderStageFlags stageFlags) : ShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stageFlags) { }
	};

	struct Descriptor
	{
		static bool createDescriptorPool(VkDescriptorPool& descriptorPool, VkDevice device, uint32_t count);
		static bool createDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout, VkDevice device, const ShaderBinding& binding);
		static bool createDescriptorSets(std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets,
			VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, const VkTexture2D& texture);

		static bool createDescriptorSets(std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets,
			VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
		static void updateDescriptorSets(std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets, VkDevice device, const BuffersUBO& ubo);
	};

	struct MemoryBuffer
	{
		static bool createVmaAllocator(VmaAllocator& vmaAllocator, const VkInstance instance, const VkPhysicalDevice physicalDevice, const VkDevice device);
		static bool allocateBufferAndMemory(VkBuffer& buffer, VmaAllocation& memRange, const VmaAllocator& vmaAllocator, uint32_t totalSizeBytes, VmaMemoryUsage memUsage, VkBufferUsageFlags flags);
		static bool createBuffer(VkBuffer& buffer, VkDeviceMemory& memory, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t bufferSize);
		static int32_t findSuitableProperties(const VkPhysicalDeviceMemoryProperties* pMemoryProperties, uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties);

		// Only works for host coherent memory type
		static bool fillMemory(VkDevice device, VkDeviceMemory memory, const void* source, uint32_t size);
	};
}
