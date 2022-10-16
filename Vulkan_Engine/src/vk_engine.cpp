#include "pch.h"
#include "Presentation/HardwareDevice.h"
#include "Presentation/Device.h"
#include "Presentation/FrameCollection.h"
#include "Presentation/Frame.h"
#include "DescriptorPoolManager.h"
#include "Presentation/PresentationTarget.h"

#include "vk_engine.h"
#include "vk_types.h"
#include "VkTypes/VulkanValidationLayers.h"
#include "EngineCore/ImGuiHandle.h"

#include "Engine/Window.h"
#include "EngineCore/Scene.h"
#include "Camera.h"
#include "VkTypes/VkShader.h"

#include "EngineCore/Material.h"

VulkanEngine::VulkanEngine() {}
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

	// Descriptor pools
	auto descPool = m_descriptorPoolManager->createNewPool(SWAPCHAIN_IMAGE_COUNT * 1000);

	VkShader::ensureDefaultShader(m_presentationDevice->getDevice());
	
	// Scene
	m_openScene = MAKEUNQ<Scene>(m_presentationDevice.get(), m_presentationTarget.get());
	if (!m_openScene->load(descPool))
	{
		printf("Failed to load the scene!");
	}

	// Camera
	m_cam = MAKEUNQ<Camera>(60.f, m_presentationTarget->getSwapchainExtent());
	// m_cam->setPosition({ -150.f, 100.f, -10.f });
	// m_cam->setRotation(0.f, 15.f);

	m_cam->setPosition({ 6.035f, 2.5f, -2.2f });
	m_cam->setRotation(-200.f, 10.f);
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
		tryInitialize<DescriptorPoolManager>(m_descriptorPoolManager, m_presentationDevice->getDevice()) &&
		tryInitialize<Presentation::PresentationTarget>(m_presentationTarget, *m_presentationHardware, *m_presentationDevice, m_window.get(), true) &&
		tryInitialize<Presentation::FrameCollection>(m_framePresentation, m_presentationDevice->getDevice(), m_presentationDevice->getCommandPool());
}

void VulkanEngine::run()
{
	SDL_Event e;
	bool quit = false;
	bool mouseHeld = false;

	float curFrameTime = static_cast<float>(SDL_GetTicks());
	float prevFrameTime = 0.f;
	float deltaTime = curFrameTime - prevFrameTime;
	prevFrameTime = curFrameTime;

	int prevMsX = -1, prevMsY = -1;
	int deltaMsX = 0, deltaMsY = 0;

	//const auto endTime = std::chrono::steady_clock::now();
	//const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
	
	// Main loop
	while (!quit)
	{
		// Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			auto& eType = e.type;
			auto& keyCode = e.key.keysym.sym;
			if (eType == SDL_QUIT || (eType == SDL_KEYDOWN && keyCode == SDLK_ESCAPE))
				quit = true;

			if (eType == SDL_KEYDOWN)
			{
				if (keyCode == SDLK_w)
					m_cam->enqueueMovement(glm::vec3(0.f, 0.f, -1.f));
				if (keyCode == SDLK_s)
					m_cam->enqueueMovement(glm::vec3(0.f, 0.f, 1.f));
				if (keyCode == SDLK_a)
					m_cam->enqueueMovement(glm::vec3(-1.f, 0.f, 0.f));
				if (keyCode == SDLK_d)
					m_cam->enqueueMovement(glm::vec3(1.f, 0.f, 0.f));
			}

			if (eType == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
			{
				mouseHeld = true;
				SDL_GetMouseState(&prevMsX, &prevMsY);
			}
			if (eType == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT)
			{
				mouseHeld = false;
			}

			if (mouseHeld)
			{
				int x, y;
				SDL_GetMouseState(&x, &y);

				deltaMsX = x - prevMsX;
				deltaMsY = y - prevMsY;
				prevMsX = x;
				prevMsY = y;
			}
		}

		if (mouseHeld)
		{
			m_cam->enqueueMouseMovement(-deltaMsX, deltaMsY);
			deltaMsX = 0;
			deltaMsY = 0;
		}
		m_cam->processFrameEvents(deltaTime);

		m_imgui->draw(m_renderLoopStatistics, m_cam.get());

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
	
	const auto renderers = m_openScene->getRenderers();
	m_renderLoopStatistics = m_presentationTarget->renderLoop(renderers, *m_cam, buffer, m_frameNumber);

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

		m_presentationTarget->releaseSwapChain(m_presentationDevice->getDevice());
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
		VkShader::releaseGlobalShaderList(m_presentationDevice->getDevice());
		
		m_imgui->release(m_presentationDevice->getDevice());

		m_openScene->release(m_presentationDevice->getDevice(), m_memoryAllocator->m_allocator);

		m_framePresentation->releaseFrameResources();

		m_descriptorPoolManager->release();

		m_presentationTarget->releaseAllResources(m_presentationDevice->getDevice());

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
