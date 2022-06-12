#include "pch.h"
#include <vk_types.h>
#include "vk_engine.h"

#include <iostream>
#include <vector>
#include <assert.h>
#include <cassert>

#include <stb_image.h>

void VulkanEngine::initializeTheWindow()
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	
	m_window = SDL_CreateWindow(
		m_applicationName.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		m_startingWindowSize.width,
		m_startingWindowSize.height,
		window_flags
	);
}

void VulkanEngine::init(bool requestValidationLayers)
{
	// Window
	initializeTheWindow();

	// Vulkan
	if (requestValidationLayers)
	{
		m_validationLayers = MAKEUNQ<VulkanValidationLayers>();
	}

	if (m_isInitialized = init_vulkan(m_window))
	{
		std::cout << "Successfully created instance." << std::endl;
	}
	else throw std::runtime_error("Failed to create and initialize vulkan objects!");

	// ImGUI
	m_imgui = MAKEUNQ<ImGuiHandle>(m_instance, m_presentationHardware->getActiveGPU(), m_presentationDevice.get(), 
		m_presentationTarget->getRenderPass(), m_framePresentation->getImageCount(), m_window);

	// Scene
	m_openScene = MAKEUNQ<Scene>();
	if (!m_openScene->load(m_memoryAllocator))
	{
		throw std::runtime_error("Failed to load the scene!");
	}

	// Camera
	m_cam = MAKEUNQ<Camera>(70.f, m_presentationTarget->getSwapchainExtent());
	m_cam->setPosition({ 0.f, 0.f, -4.f });
	m_cam->setRotation({ -0.2f, -0.66f, 0.12f });

	// Texture
	m_texture = MAKEUNQ<VkTexture>();
	if (!Texture::loadImage(*m_texture, "C:/Git/Vulkan_Engine/Resources/vulkan_tutorial_texture.jpg", m_memoryAllocator, m_presentationDevice.get()))
	{
		printf("Failed to create VKTexture\n");
	}

	/*		========================== DESCRIPTOR POOL ========================		*/
	auto imageCount = m_framePresentation->getImageCount();
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = imageCount;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = imageCount;
	if (vkCreateDescriptorPool(m_presentationDevice->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}

	/*		====================== Descriptor set Layout ====================		*/
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

	if (vkCreateDescriptorSetLayout(m_presentationDevice->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	/*		====================== Graphics Pipeline ====================		*/
	m_presentationTarget->createGraphicsPipeline(m_presentationDevice->getDevice(), m_openScene->getVertexBinding(), descriptorSetLayout);

	/*		======================= Descriptor set =====================		*/
	std::vector<VkDescriptorSetLayout> layouts(imageCount, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = imageCount;
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(imageCount);
	if (vkAllocateDescriptorSets(m_presentationDevice->getDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < imageCount; i++) 
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_texture->imageView;
		imageInfo.sampler = m_texture->sampler;

		VkWriteDescriptorSet descriptorWrite{};

		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_presentationDevice->getDevice(), 1, &descriptorWrite, 0, nullptr);
	}
}

bool VulkanEngine::init_vulkan(SDL_Window* window)
{
	if (m_validationLayers && m_validationLayers->checkValidationLayersFailed())
		throw std::runtime_error("validation layers requested, but not available!");

	unsigned int extCount = 0;
	if (!vkinit::Instance::getRequiredExtensionsForPlatform(window, &extCount, nullptr))
		return false;

	// SDL requires { "VK_KHR_surface", "VK_KHR_win32_surface" } extensions for windows
	std::vector<const char*> extensions(extCount);
	if (!vkinit::Instance::getRequiredExtensionsForPlatform(window, &extCount, extensions.data()))
		return false;

	VkSurfaceKHR surface;
	if (!vkinit::Instance::createInstance(m_instance, m_applicationName, extensions, m_validationLayers.get()) ||
		!vkinit::Surface::createSurface(surface, m_instance, window))
		return false;

	m_presentationHardware = MAKEUNQ<Presentation::HardwareDevice>(m_instance, surface);
	m_presentationDevice = MAKEUNQ<Presentation::Device>(m_presentationHardware->getActiveGPU(), surface, window, m_validationLayers.get());

	m_presentationTarget = MAKEUNQ<Presentation::PresentationTarget>(*m_presentationHardware, *m_presentationDevice);
	m_framePresentation = MAKEUNQ<Presentation::FrameCollection>(m_presentationDevice->getDevice(), m_presentationDevice->getCommandPool());

	if (!vkinit::MemoryBuffer::createVmaAllocator(m_memoryAllocator, m_instance, m_presentationHardware->getActiveGPU(), m_presentationDevice->getDevice()))
		return false;
	
	return m_presentationHardware->isInitialized() &&
		m_presentationDevice->isInitialized() &&
		m_presentationTarget->isInitialized() &&
		m_framePresentation->isInitialized();
}

void VulkanEngine::run()
{
	SDL_Event e;
	bool quit = false;

	// Main loop
	while (!quit)
	{
		// Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			// close the window when user alt-f4s or clicks the X button			
			if (e.type == SDL_QUIT) quit = true;
		}

		m_imgui->draw(m_window, m_cam.get());

		draw();
	}

	// Wait until all scheduled operations are completed before releasing resources.
	vkDeviceWaitIdle(m_presentationDevice->getDevice());
}

void VulkanEngine::draw()
{
	auto frame = m_framePresentation->getNextFrameAndWaitOnFence();

	uint32_t imageIndex;
	auto result = m_framePresentation->acquireImageFromSwapchain(imageIndex, m_presentationTarget->getSwapchain());
	
	if (handleFailedToAcquireImageIfNecessary(result))
		return;

	auto buffer = frame.getCommandBuffer();
	vkResetCommandBuffer(buffer, 0);
	
	CommandObjectsWrapper::renderIndexedMeshes(buffer, m_presentationTarget->getPipeline(), m_presentationTarget->getPipelineLayout(), m_presentationTarget->getRenderPass(),
		m_presentationTarget->getSwapchainFrameBuffers(imageIndex), m_presentationTarget->getSwapchainExtent(), *m_cam, m_openScene->getGraphicsMeshes(), descriptorSets, *m_texture, m_frameNumber);

	frame.resetAcquireFence(m_presentationDevice->getDevice());
	frame.submitToQueue(m_presentationDevice->getGraphicsQueue());
	frame.present(imageIndex, m_presentationTarget->getSwapchain(), m_presentationDevice->getPresentQueue());

	++m_frameNumber;
}

bool VulkanEngine::handleFailedToAcquireImageIfNecessary(VkResult imageAcquireResult)
{
	if (imageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		vkDeviceWaitIdle(m_presentationDevice->getDevice());

		m_presentationTarget->release(m_presentationDevice->getDevice());
		if (!m_presentationTarget->createPresentationTarget(*m_presentationHardware, *m_presentationDevice))
			printf("Recreating the swapchain was not successful");

		return true;
	}
	else if (imageAcquireResult != VK_SUCCESS && imageAcquireResult != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	return false;
}

void VulkanEngine::cleanup()
{
	if (m_instance)
	{
		vkDestroySampler(m_presentationDevice->getDevice(), m_texture->sampler, nullptr);
		vkDestroyImageView(m_presentationDevice->getDevice(), m_texture->imageView, nullptr);
		vmaDestroyImage(m_memoryAllocator, m_texture->image, m_texture->memoryRange);

		m_imgui->release(m_presentationDevice->getDevice());

		m_openScene->release(m_memoryAllocator);

		vmaDestroyAllocator(m_memoryAllocator);

		m_framePresentation->releaseFrameResources();

		vkDestroyDescriptorPool(m_presentationDevice->getDevice(), descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_presentationDevice->getDevice(), descriptorSetLayout, nullptr);
		m_presentationTarget->releasePipeline(m_presentationDevice->getDevice());

		m_presentationTarget->release(m_presentationDevice->getDevice());
		m_presentationDevice->release();

		vkDestroySurfaceKHR(m_instance, m_presentationDevice->getSurface(), nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	if (m_window)
	{
		SDL_DestroyWindow(m_window);
	}
}