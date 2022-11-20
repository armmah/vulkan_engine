#include "pch.h"
#include "Engine/Window.h"
#include "InitializersUtility.h"
#include "VkTexture.h"
#include "BuffersUBO.h"

bool vkinit::Surface::createSurface(VkSurfaceKHR& surface, VkInstance instance, const Window* window)
{
	return SDL_Vulkan_CreateSurface(window->get(), instance, &surface);
}

struct RenderPassAttachement
{
	enum LoadTransitionState
	{
		Preserve	= VK_ATTACHMENT_LOAD_OP_LOAD,
		Clear		= VK_ATTACHMENT_LOAD_OP_CLEAR,
		Undefined	= VK_ATTACHMENT_LOAD_OP_DONT_CARE
	};
	enum StoreTransitionState
	{
		Store		= VK_ATTACHMENT_STORE_OP_STORE,
		Discard		= VK_ATTACHMENT_STORE_OP_DONT_CARE
	};

	RenderPassAttachement(VkFormat format, LoadTransitionState loadState, StoreTransitionState storeState,
		VkImageLayout initialLayout, VkImageLayout finalLayout,
		VkSampleCountFlagBits sameplCount = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT)
	{
		m_attachement = {};
		m_attachement.format = format;
		m_attachement.samples = sameplCount;

		m_attachement.loadOp = getLoadTransitionState(loadState);
		m_attachement.storeOp = getStoreTransitionState(storeState);

		m_attachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		m_attachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		m_attachement.initialLayout = initialLayout;
		m_attachement.finalLayout = finalLayout;
	}

	const VkAttachmentDescription getAttachement() const { return m_attachement; }

private:
	VkAttachmentDescription m_attachement;

	static constexpr VkAttachmentLoadOp getLoadTransitionState(LoadTransitionState state) { return static_cast<VkAttachmentLoadOp>(state); }
	static constexpr VkAttachmentStoreOp getStoreTransitionState(StoreTransitionState state) { return static_cast<VkAttachmentStoreOp>(state); }
};

struct ColorAttachement : RenderPassAttachement
{
	ColorAttachement(VkFormat format, LoadTransitionState loadState = LoadTransitionState::Clear, StoreTransitionState storeState = StoreTransitionState::Store,
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VkSampleCountFlagBits sameplCount = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT)
		: RenderPassAttachement(format, loadState, storeState, initialLayout, finalLayout, sameplCount)
	{ }
};

struct DepthAttachment : RenderPassAttachement
{
	static constexpr VkFormat FORMAT = VK_FORMAT_D32_SFLOAT;

	DepthAttachment(VkFormat format = FORMAT, LoadTransitionState loadState = LoadTransitionState::Clear, StoreTransitionState storeState = StoreTransitionState::Store,
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VkSampleCountFlagBits sameplCount = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT)
		: RenderPassAttachement(format, loadState, storeState, initialLayout, finalLayout, sameplCount)
	{ }
};

bool vkinit::Surface::createRenderPass(VkRenderPass& renderPass, VkDevice device, VkFormat swapchainImageFormat, bool enableDepthAttachment)
{
	auto enableColorAttachment = swapchainImageFormat != VK_FORMAT_UNDEFINED;
	assert(enableColorAttachment || enableDepthAttachment);

	auto attachementCount = 0;

	VkPipelineStageFlags srcStageBit = 0;
	VkPipelineStageFlags dstStageBit = 0;
	VkAccessFlags dstAccessBit = 0;

	std::array<VkAttachmentDescription, 2> attachments = { };

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkAttachmentReference colorAttachmentRef{};
	if (enableColorAttachment)
	{
		colorAttachmentRef.attachment = attachementCount;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		srcStageBit |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dstStageBit |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dstAccessBit |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		attachments[attachementCount] = ColorAttachement(swapchainImageFormat).getAttachement();
		attachementCount += 1;
	}

	VkAttachmentReference depthAttachment{};
	if (enableDepthAttachment)
	{
		depthAttachment.attachment = attachementCount;
		depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		subpass.pDepthStencilAttachment = &depthAttachment;

		srcStageBit |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dstStageBit |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dstAccessBit |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		auto depthAttachmentCtor = enableColorAttachment ? DepthAttachment() : 
			DepthAttachment(DepthAttachment::FORMAT, RenderPassAttachement::LoadTransitionState::Clear, RenderPassAttachement::StoreTransitionState::Store, 
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		attachments[attachementCount] = depthAttachmentCtor.getAttachement();
		attachementCount += 1;
	}

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachementCount;
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkSubpassDependency dependency;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = srcStageBit;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = dstStageBit;
	dependency.dstAccessMask = dstAccessBit;
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	renderPassInfo.dependencyCount = 1u;
	renderPassInfo.pDependencies = &dependency;

	return vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS;
}

bool vkinit::Descriptor::createDescriptorPool(VkDescriptorPool& descriptorPool, VkDevice device, uint32_t count)
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

bool vkinit::Descriptor::createDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout, VkDevice device, const ShaderBinding& binding)
{
	auto layoutBinding = binding.getLayoutBinding();

	VkDescriptorSetLayoutCreateInfo layoutCI{};
	layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCI.bindingCount = 1;
	layoutCI.pBindings = &layoutBinding;

	return vkCreateDescriptorSetLayout(device, &layoutCI, nullptr, &descriptorSetLayout) == VK_SUCCESS;
}

bool vkinit::Descriptor::createDescriptorSets(std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets, VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout)
{
	std::array<VkDescriptorSetLayout, SWAPCHAIN_IMAGE_COUNT> layouts{};
	for (auto& l : layouts)
	{
		l = descriptorSetLayout;
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = SWAPCHAIN_IMAGE_COUNT;
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		printf("Failed to allocate descriptor sets!\n");
		return false;
	}

	return true;
}

void vkinit::Descriptor::updateDescriptorSets(std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets, VkDevice device, const BuffersUBO& ubo)
{
	std::array<VkWriteDescriptorSet, SWAPCHAIN_IMAGE_COUNT> descriptorWrites{};
	std::array<VkDescriptorBufferInfo, SWAPCHAIN_IMAGE_COUNT> bufferInfo{};
	for (size_t i = 0; i < SWAPCHAIN_IMAGE_COUNT; i++)
	{
		bufferInfo[i].buffer = ubo.buffers[i];
		bufferInfo[i].offset = 0;
		bufferInfo[i].range = ubo.byteSize;

		descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[i].dstSet = descriptorSets[i];
		descriptorWrites[i].dstBinding = 0;
		descriptorWrites[i].dstArrayElement = 0;
		descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[i].descriptorCount = 1;
		descriptorWrites[i].pBufferInfo = &bufferInfo[i];
	}
	vkUpdateDescriptorSets(device, SWAPCHAIN_IMAGE_COUNT, descriptorWrites.data(), 0, nullptr);
}

bool vkinit::Descriptor::createDescriptorSets(std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets,
	VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, const VkTexture2D& texture)
{
	return vkinit::Descriptor::createDescriptorSets(descriptorSets, device, descriptorPool, descriptorSetLayout, texture.imageView, texture.sampler);
}

bool vkinit::Descriptor::createDescriptorSets(std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets,
	VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, const VkImageView& imageView, const VkSampler& sampler)
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
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		printf("Failed to allocate descriptor sets!\n");
		return false;
	}

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = imageView;
	imageInfo.sampler = sampler;

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
	vkUpdateDescriptorSets(device, imageCount, descriptorWrites.data(), 0, nullptr);

	return true;
}

bool vkinit::Commands::createSingleCommandBuffer(VkCommandBuffer& commandBuffer, VkCommandPool pool, VkDevice device)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	return vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) == VK_SUCCESS;
}

bool vkinit::Commands::createCommandBuffers(std::vector<VkCommandBuffer>& commandBufferCollection, uint32_t count, VkCommandPool pool, VkDevice device)
{
	commandBufferCollection.resize(count);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = count;

	return vkAllocateCommandBuffers(device, &allocInfo, commandBufferCollection.data()) == VK_SUCCESS;
}

void vkinit::Commands::initViewportAndScissor(VkViewport& viewport, VkRect2D& scissor, VkExtent2D extent, int32_t offsetX, int32_t offsetY)
{
	viewport.x = static_cast<float>( offsetX );
	viewport.y = static_cast<float>( offsetY );
	viewport.width = static_cast<float>( extent.width );
	viewport.height = static_cast<float>( extent.height );
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor.offset = { offsetX, offsetY };
	scissor.extent = extent;
}

bool vkinit::Synchronization::createSemaphore(VkSemaphore& semaphore, VkDevice device)
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	return vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) == VK_SUCCESS;
}

bool vkinit::Synchronization::createFence(VkFence& fence, VkDevice device, bool createSignaled)
{
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	if (createSignaled)
	{
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	}

	return vkCreateFence(device, &fenceInfo, nullptr, &fence) == VK_SUCCESS;
}

bool vkinit::Texture::createSwapchain(VkSwapchainKHR& swapchain, VkDevice device, VkSurfaceKHR surface, uint32_t imageCount, VkExtent2D extent, VkPresentModeKHR presentationMode, VkSurfaceFormatKHR surfaceFormat, VkSurfaceTransformFlagBitsKHR currentTransform)
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

bool vkinit::Texture::createImage(VkImage& image, VmaAllocation& memoryRange, const MemAllocationInfo& allocInfo, VkFormat format, VkImageUsageFlags usageFlags, uint32_t width, uint32_t height, uint32_t mipCount)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;

	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;

	imageInfo.mipLevels = mipCount;
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

bool vkinit::Texture::createTextureImageView(VkImageView& imageView, VkDevice device, VkImage image, VkFormat format, uint32_t mipCount, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipCount;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	return vkCreateImageView(device, &viewInfo, nullptr, &imageView) == VK_SUCCESS;
}

bool vkinit::Texture::createTextureSampler(VkSampler& sampler, VkDevice device, uint32_t mipCount, bool linearFiltering, VkSamplerAddressMode sampleMode, 
	float anisotropySamples, std::optional<VkCompareOp> compareOp)
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

	samplerInfo.compareEnable = compareOp.has_value() ? VK_TRUE : VK_FALSE;
	samplerInfo.compareOp = compareOp.has_value() ? compareOp.value() : VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(mipCount);

	return (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) == VK_SUCCESS);
}

bool vkinit::MemoryBuffer::createVmaAllocator(VmaAllocator& vmaAllocator, const VkInstance instance, const VkPhysicalDevice physicalDevice, const VkDevice device)
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.instance = instance;
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device;

	return vmaCreateAllocator(&allocatorInfo, &vmaAllocator) == VK_SUCCESS;
}

bool vkinit::MemoryBuffer::allocateBufferAndMemory(VkBuffer& buffer, VmaAllocation& memRange, const VmaAllocator& vmaAllocator, uint32_t totalSizeBytes, VmaMemoryUsage memUsage, VkBufferUsageFlags flags)
{
	VmaAllocationCreateInfo vmaACI{};
	vmaACI.usage = memUsage;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.usage = flags;
	bufferInfo.size = totalSizeBytes;

	return vmaCreateBuffer(vmaAllocator, &bufferInfo, &vmaACI, &buffer, &memRange, nullptr) == VK_SUCCESS;
}

bool vkinit::MemoryBuffer::createBuffer(VkBuffer& buffer, VkDeviceMemory& memory, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t bufferSize)
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

int32_t vkinit::MemoryBuffer::findSuitableProperties(const VkPhysicalDeviceMemoryProperties* pMemoryProperties, uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties)
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
bool vkinit::MemoryBuffer::fillMemory(VkDevice device, VkDeviceMemory memory, const void* source, uint32_t size)
{
	void* data;
	vkMapMemory(device, memory, 0, size, 0, &data);
	memcpy(data, source, static_cast<size_t>(size));
	vkUnmapMemory(device, memory);

	return true;
}
