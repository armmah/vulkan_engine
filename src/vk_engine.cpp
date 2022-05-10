#include <iostream>

#include <vk_types.h>
#include "vk_engine.h"

#include <vector>
#include <assert.h>
#include <cassert>

void VulkanEngine::init()
{
	// We initialize SDL and create a window with it. 
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	
	m_window = SDL_CreateWindow(
		"Vulkan Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		m_startingWindowSize.width,
		m_startingWindowSize.height,
		window_flags
	);

	if (_isInitialized = init_vulkan(m_window))
	{
		std::cout << "Successfully created instance." << std::endl;
	}
	else
	{
		throw std::runtime_error("Failed to create and initialize vulkan objects!");
	}
}

bool VulkanEngine::init_vulkan(SDL_Window* window)
{
	if (m_validationLayers.checkValidationLayersFailed())
		throw std::runtime_error("validation layers requested, but not available!");

	unsigned int extCount = 0;
	if (!vkinit::Instance::getRequiredExtensionsForPlatform(window, &extCount, nullptr))
		return false;

	// SDL requires { "VK_KHR_surface", "VK_KHR_win32_surface" } extensions for windows
	std::vector<const char*> extensions(extCount);
	if (!vkinit::Instance::getRequiredExtensionsForPlatform(window, &extCount, extensions.data()))
		return false;

	VkSurfaceKHR surface;
	bool instanceCreated = 
		vkinit::Instance::createInstance(m_instance, applicationName, extensions, m_validationLayers) &&
		vkinit::Surface::createSurface(surface, m_instance, window);

	m_presentationHardware = std::make_shared<HardwarePresentationDevice>(m_instance, surface);
	m_presentationDevice = std::make_shared<PresentationDevice>(m_presentationHardware->getActiveGPU(), surface, window, m_validationLayers);
	m_swapchain = std::make_unique<PresentationTarget>(m_presentationHardware, m_presentationDevice);
	m_framePresentation = std::make_unique<PresentationFrames>(m_presentationDevice);

	return
		instanceCreated &&
		m_presentationHardware->isInitialized() &&
		m_presentationDevice->isInitialized() &&
		m_swapchain->isInitialized() &&
		m_framePresentation->isInitialized();
}

void VulkanEngine::run()
{
	SDL_Event e;
	bool bQuit = false;

	//main loop
	while (!bQuit)
	{
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//close the window when user alt-f4s or clicks the X button			
			if (e.type == SDL_QUIT) bQuit = true;
		}

		draw();
	}

	// Wait until all scheduled operations are completed before releasing resources.
	vkDeviceWaitIdle(m_presentationDevice->getDevice());
}

void VulkanEngine::draw()
{
	auto frame = m_framePresentation->getNextFrameAndWaitOnFence();

	uint32_t imageIndex;
	auto result = m_framePresentation->acquireImageFromSwapchain(imageIndex, m_swapchain->swapchain);
	
	if (handleFailedToAcquireImageIfNecessary(result))
		return;

	auto buffer = frame.getCommandBuffer();
	vkResetCommandBuffer(buffer, 0);
	
	CommandObjectsWrapper::HelloTriangleCommand(buffer, m_swapchain->pipeline, m_swapchain->renderPass, m_swapchain->swapChainFrameBuffers[imageIndex], m_swapchain->swapChainExtent);

	frame.resetAcquireFence(m_presentationDevice->getDevice());
	frame.submitToQueue(m_presentationDevice->getGraphicsQueue());
	frame.present(imageIndex, m_swapchain->swapchain, m_presentationDevice->getPresentQueue());
}

bool VulkanEngine::handleFailedToAcquireImageIfNecessary(VkResult imageAcquireResult)
{
	if (imageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		vkDeviceWaitIdle(m_presentationDevice->getDevice());

		m_swapchain->release(m_presentationDevice->getDevice());
		if (!m_swapchain->createPresentationTarget(m_presentationHardware, m_presentationDevice))
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
		m_framePresentation->releaseFrameResources();

		m_swapchain->release(m_presentationDevice->getDevice());
		m_presentationDevice->release();

		vkDestroySurfaceKHR(m_instance, m_presentationDevice->getSurface(), nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	if (m_window)
	{
		SDL_DestroyWindow(m_window);
	}
}