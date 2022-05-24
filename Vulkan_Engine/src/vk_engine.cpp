#include <iostream>

#include <vk_types.h>
#include "vk_engine.h"

#include <vector>
#include <assert.h>
#include <cassert>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

void VulkanEngine::initializeTheWindow()
{
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
}

void VulkanEngine::initImGui()
{
	const uint32_t DESC_POOL_SIZE = 1000u;
	auto device = m_presentationDevice->getDevice();

	//1: create descriptor pool for IMGUI
	// the size of the pool is very oversize, but it's copied from imgui demo itself.
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, DESC_POOL_SIZE }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = DESC_POOL_SIZE;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	vkCreateDescriptorPool(device, &pool_info, nullptr, &m_imguiPool);

	// 2: initialize imgui library

	//this initializes the core structures of imgui
	ImGui::CreateContext();

	//this initializes imgui for SDL
	ImGui_ImplSDL2_InitForVulkan(m_window);

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_instance;
	init_info.PhysicalDevice = m_presentationHardware->getActiveGPU();
	init_info.Device = device;
	init_info.Queue = m_presentationDevice->getGraphicsQueue();
	init_info.DescriptorPool = m_imguiPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, m_presentationTarget->getRenderPass());

	{
		auto cmdPool = m_presentationDevice->getCommandPool();
		VkCommandBuffer cmdBuffer;
		vkinit::Commands::createSingleCommandBuffer(cmdBuffer, cmdPool, device);

		VkFence fence;
		vkinit::Synchronization::createFence(fence, device, false);

		//execute a gpu command to upload imgui font textures
		{
			CommandObjectsWrapper::CommandBufferScope sc(cmdBuffer);
			ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;
		vkQueueSubmit(m_presentationDevice->getGraphicsQueue(), 1, &submitInfo, fence);

		vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
		vkDestroyFence(device, fence, nullptr);
	}

	//clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void releaseImGui(VkDevice device, VkDescriptorPool descriptorPool)
{
	//add the destroy the imgui created structures
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	ImGui_ImplVulkan_Shutdown();
}

void VulkanEngine::init(bool requestValidationLayers)
{
	initializeTheWindow();

	if (requestValidationLayers)
	{
		m_validationLayers = MAKEUNQ<VulkanValidationLayers>();
	}

	if (_isInitialized = init_vulkan(m_window))
	{
		std::cout << "Successfully created instance." << std::endl;
	}
	else throw std::runtime_error("Failed to create and initialize vulkan objects!");

	initImGui();

	m_openScene = MAKEUNQ<Scene>();
	if (!m_openScene->load(m_memoryAllocator))
	{
		throw std::runtime_error("Failed to load the scene!");
	}
	m_presentationTarget->createGraphicsPipeline(m_presentationDevice->getDevice(), m_openScene->getVertexBinding());

	m_cam = MAKEUNQ<Camera>(70.f, m_presentationTarget->getSwapchainExtent());
	m_cam->setPosition({ 0.f, 0.f, -4.f });
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
	if (!vkinit::Instance::createInstance(m_instance, applicationName, extensions, m_validationLayers.get()) ||
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

		//imgui new frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame(m_window);

		ImGui::NewFrame();

		ImGui::Begin("Camera controls");
		auto rot = glm::eulerAngles(m_cam->getRotation());
		auto pi_half = static_cast<float>(M_PI) / 2.0f;

		ImGui::SliderFloat("cam_rot_x", &rot.x, -pi_half, pi_half);
		ImGui::SliderFloat("cam_rot_y", &rot.y, -pi_half, pi_half);
		ImGui::SliderFloat("cam_rot_z", &rot.z, -pi_half, pi_half);
		m_cam->setRotation(rot);

		ImGui::End();

		draw();
	}

	// Wait until all scheduled operations are completed before releasing resources.
	vkDeviceWaitIdle(m_presentationDevice->getDevice());
}

void VulkanEngine::draw()
{
	ImGui::Render();

	auto frame = m_framePresentation->getNextFrameAndWaitOnFence();

	uint32_t imageIndex;
	auto result = m_framePresentation->acquireImageFromSwapchain(imageIndex, m_presentationTarget->getSwapchain());
	
	if (handleFailedToAcquireImageIfNecessary(result))
		return;

	auto buffer = frame.getCommandBuffer();
	vkResetCommandBuffer(buffer, 0);
	
	CommandObjectsWrapper::renderIndexedMeshes(buffer, m_presentationTarget->getPipeline(), m_presentationTarget->getPipelineLayout(), m_presentationTarget->getRenderPass(),
		m_presentationTarget->getSwapchainFrameBuffers(imageIndex), m_presentationTarget->getSwapchainExtent(), *m_cam, m_openScene->getGraphicsMeshes(), m_frameNumber);

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
		releaseImGui(m_presentationDevice->getDevice(), m_imguiPool);

		m_openScene->release(m_memoryAllocator);

		vmaDestroyAllocator(m_memoryAllocator);

		m_framePresentation->releaseFrameResources();

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