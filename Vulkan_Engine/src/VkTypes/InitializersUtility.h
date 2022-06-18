#pragma once
#include "pch.h"
#include "Common.h"
#include "VulkanValidationLayers.h"
#include "VkTexture.h"
#include "Presentation/Device.h"
#include "VkMemoryAllocator.h"

class Window;

namespace vkinit
{

	struct Instance
	{
		static const std::vector<const char*> requiredExtensions;

		static bool getRequiredExtensionsForPlatform(Window const* window, unsigned int* extCount, const char** extensionNames);
		static bool createInstance(VkInstance& instance, std::string applicationName, std::vector<const char*> extNames, const VulkanValidationLayers* validationLayers);
	};

	struct Surface
	{
		static bool createSurface(VkSurfaceKHR& surface, VkInstance instance, Window* window);
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

	struct Texture
	{
		static bool createSwapchain(VkSwapchainKHR& swapchain, VkDevice device, VkSurfaceKHR surface, uint32_t imageCount, VkExtent2D extent,
			VkPresentModeKHR presentationMode, VkSurfaceFormatKHR surfaceFormat, VkSurfaceTransformFlagBitsKHR currentTransform)
		{
			VkSwapchainCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = surface;
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.preTransform = currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentationMode;
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = VK_NULL_HANDLE;

			return vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) == VK_SUCCESS;
		}

		static bool createImage(VkImage& image, VmaAllocation& memoryRange, const MemAllocationInfo& allocInfo, VkFormat format, VkImageUsageFlags usageFlags, uint32_t width, uint32_t height)
		{
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;

			imageInfo.extent.width = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth = 1;

			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = usageFlags;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.flags = 0;

			VmaAllocationCreateInfo aci = allocInfo.getAllocationCreateInfo();

			return vmaCreateImage(*allocInfo.allocator, &imageInfo, &aci, &image, &memoryRange, nullptr) == VK_SUCCESS;
		}

		static bool createTextureImageView(VkImageView& imageView, VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT)
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = image;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = format;
			viewInfo.subresourceRange.aspectMask = aspectFlags;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			return vkCreateImageView(device, &viewInfo, nullptr, &imageView) == VK_SUCCESS;
		}

		static bool createTextureSampler(VkSampler& sampler, VkDevice device, bool linearFiltering = true, VkSamplerAddressMode sampleMode = VK_SAMPLER_ADDRESS_MODE_REPEAT, float anisotropySamples = 0.0f)
		{
			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

			auto filterMode = linearFiltering ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
			samplerInfo.magFilter = filterMode;
			samplerInfo.minFilter = filterMode;

			samplerInfo.addressModeU = sampleMode;
			samplerInfo.addressModeV = sampleMode;
			samplerInfo.addressModeW = sampleMode;

			samplerInfo.anisotropyEnable = anisotropySamples > 0;
			samplerInfo.maxAnisotropy = anisotropySamples;

			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = 0.0f;

			return (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) == VK_SUCCESS);
		}
	};

	struct Descriptor
	{
		static bool createDescriptorPool(VkDescriptorPool& descriptorPool, VkDevice device, uint32_t count)
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = count;

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = 1;
			poolInfo.pPoolSizes = &poolSize;
			poolInfo.maxSets = count;

			return vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) == VK_SUCCESS;
		}

		static bool createDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout, VkDevice device)
		{
			VkDescriptorSetLayoutBinding samplerLayoutBinding{};
			samplerLayoutBinding.binding = 0;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerLayoutBinding.pImmutableSamplers = nullptr;
			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &samplerLayoutBinding;

			return vkCreateDescriptorSetLayout(device, & layoutInfo, nullptr, &descriptorSetLayout) == VK_SUCCESS;
		}

		static bool createDescriptorSets(std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets,
			VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, const VkTexture2D& texture)
		{
			constexpr auto imageCount = SWAPCHAIN_IMAGE_COUNT;
			std::array<VkDescriptorSetLayout, imageCount> layouts{};
			for (auto& l : layouts)
			{
				l = descriptorSetLayout;
			}

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = imageCount;
			allocInfo.pSetLayouts = &layouts[0];

			if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
			{
				printf("Failed to allocate descriptor sets!\n");
				return false;
			}

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texture.imageView;
			imageInfo.sampler = texture.sampler;

			std::array<VkWriteDescriptorSet, imageCount> descriptorWrites{};
			for (size_t i = 0; i < imageCount; i++)
			{
				descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[i].dstSet = descriptorSets[i];
				descriptorWrites[i].dstBinding = 0;
				descriptorWrites[i].dstArrayElement = 0;
				descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[i].descriptorCount = 1;
				descriptorWrites[i].pImageInfo = &imageInfo;

			}
			vkUpdateDescriptorSets(device, imageCount, &descriptorWrites[0], 0, nullptr);

			return true;
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

		static bool allocateBufferAndMemory(VkBuffer& buffer, VmaAllocation& memRange, const VmaAllocator& vmaAllocator, uint32_t totalSizeBytes, VmaMemoryUsage memUsage, VkBufferUsageFlags flags)
		{
			VmaAllocationCreateInfo vmaACI{};
			vmaACI.usage = memUsage;

			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.usage = flags;
			bufferInfo.size = totalSizeBytes;

			return vmaCreateBuffer(vmaAllocator, &bufferInfo, &vmaACI, &buffer, &memRange, nullptr) == VK_SUCCESS;
		}

		static bool createBuffer(VkBuffer& buffer, VkDeviceMemory& memory, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t bufferSize)
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = bufferSize;

			bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
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
