﻿#include "pch.h"
#include "Presentation/HardwareDevice.h"
#include "Presentation/Device.h"
#include "Presentation/FrameCollection.h"
#include "Presentation/Frame.h"
#include "Presentation/PresentationTarget.h"

#include "vk_types.h"
#include "VkTypes/VulkanValidationLayers.h"
#include "EngineCore/ImGuiHandle.h"

#include "Engine/Window.h"
#include "EngineCore/Scene.h"
#include "Camera.h"

#include <EngineCore/Material.h>
#include "vk_engine.h"

VulkanEngine::VulkanEngine() { }
VulkanEngine::~VulkanEngine() { }

void VulkanEngine::init(bool requestValidationLayers)
{
	// Window
	m_window = MAKEUNQ<Window>(static_cast<SDL_WindowFlags>(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE), m_applicationName, m_startingWindowSize.width, m_startingWindowSize.height);

	// Vulkan
	if (requestValidationLayers)
	{
		m_validationLayers = MAKEUNQ<VulkanValidationLayers>();
	}

	if (m_isInitialized = init_vulkan())
	{
		std::cout << "Successfully created instance." << std::endl;
	}
	else throw std::runtime_error("Failed to create and initialize vulkan objects!");

	// ImGUI
	m_imgui = MAKEUNQ<ImGuiHandle>(m_instance, m_presentationHardware->getActiveGPU(), m_presentationDevice.get(), 
		m_presentationTarget->getRenderPass(), m_framePresentation->getImageCount(), m_window.get());

	// Scene
	m_openScene = MAKEUNQ<Scene>();
	if (!m_openScene->load(m_memoryAllocator->m_allocator))
	{
		throw std::runtime_error("Failed to load the scene!");
	}

	// Camera
	m_cam = MAKEUNQ<Camera>(70.f, m_presentationTarget->getSwapchainExtent());
	m_cam->setPosition({ 0.f, 0.f, -3.f });
	m_cam->setRotation({ -0.2f, -0.66f, 0.12f });

	// Descriptor pools
	m_descriptorPoolManager = MAKEUNQ<DescriptorPoolManager>(m_presentationDevice->getDevice());
	auto descPool = m_descriptorPoolManager->createNewPool(SWAPCHAIN_IMAGE_COUNT);

	// Material (texture & shader)
	MaterialSource msrc = {};
	m_material = MAKEUNQ<Material>(m_presentationDevice.get(), m_presentationTarget.get(), descPool, msrc);
}

bool VulkanEngine::init_vulkan()
{
	if (m_validationLayers && m_validationLayers->checkValidationLayersFailed())
		throw std::runtime_error("Validation layers requested, but not available!");

	VkSurfaceKHR surface;
	if (!vkinit::Instance::createInstance(m_instance, m_window.get(), m_validationLayers.get()) ||
		!vkinit::Surface::createSurface(surface, m_instance, m_window.get()))
		return false;

	return tryInitialize<Presentation::HardwareDevice>(m_presentationHardware, m_instance, surface) &&
		tryInitialize<Presentation::Device>(m_presentationDevice, m_presentationHardware->getActiveGPU(), surface, m_window.get(), m_validationLayers.get()) &&
		tryInitialize<VkMemoryAllocator>(m_memoryAllocator, m_instance, m_presentationHardware->getActiveGPU(), m_presentationDevice->getDevice()) &&
		tryInitialize<Presentation::PresentationTarget>(m_presentationTarget, *m_presentationHardware, *m_presentationDevice, m_window.get(), true) &&
		tryInitialize<Presentation::FrameCollection>(m_framePresentation, m_presentationDevice->getDevice(), m_presentationDevice->getCommandPool());
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

		m_imgui->draw(m_cam.get());

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
	
	CommandObjectsWrapper::renderIndexedMeshes(buffer, m_presentationTarget->getRenderPass(), m_presentationTarget->getSwapchainFrameBuffers(imageIndex),
		m_presentationTarget->getSwapchainExtent(), *m_cam, m_openScene->getGraphicsMeshes(), *m_material->variant.get(), m_frameNumber);

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
		m_imgui->release(m_presentationDevice->getDevice());

		m_material->release(m_presentationDevice->getDevice());

		m_openScene->release(m_memoryAllocator->m_allocator);

		m_framePresentation->releaseFrameResources();

		m_descriptorPoolManager->release();

		m_presentationTarget->release(m_presentationDevice->getDevice());

		vmaDestroyAllocator(m_memoryAllocator->m_allocator);
		m_presentationDevice->release();

		vkDestroySurfaceKHR(m_instance, m_presentationDevice->getSurface(), nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	if (m_window)
	{
		m_window->release();
	}
}