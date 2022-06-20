#include "pch.h"
#include "vk_types.h"
#include "vk_engine.h"

#include "ShaderSource.h"

#include "Scene.h"
#include "Mesh.h"
#include "VkTypes/InitializersUtility.h"
#include "VkTypes/VulkanValidationLayers.h"
#include "VkTypes/VkTexture.h"

struct TextureSource
{
	std::string path;
	// Texture type
	// Texture format
	// Sampler type
	// Other params
};

struct VkMaterialVariant
{
	//const Material* sourceMat;

	VkTexture texture;


	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

// MaterialResourceManager
class Descriptor
{

};

class Material
{

private:
	ShaderSource shaderSourcesOnDisk;
	TextureSource textureOnDisk;
};

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

	// Texture
	m_texture = MAKEUNQ<VkTexture2D>();
	if (!Texture::loadImage(*m_texture, "C:/Git/Vulkan_Engine/Resources/vulkan_tutorial_texture.jpg", m_memoryAllocator->m_allocator, m_presentationDevice.get()))
	{
		printf("Failed to create VKTexture\n");
	}

	constexpr auto imageCount = SWAPCHAIN_IMAGE_COUNT;
	vkinit::Descriptor::createDescriptorPool(descriptorPool, m_presentationDevice->getDevice(), imageCount);
	vkinit::Descriptor::createDescriptorSetLayout(descriptorSetLayout, m_presentationDevice->getDevice());
	m_presentationTarget->createGraphicsPipeline(m_presentationDevice->getDevice(), Mesh::defaultVertexBinding, descriptorSetLayout, VK_CULL_MODE_BACK_BIT, m_presentationTarget->hasDepthAttachement());
	vkinit::Descriptor::createDescriptorSets(descriptorSets, m_presentationDevice->getDevice(), descriptorPool, descriptorSetLayout, *m_texture);
}

bool VulkanEngine::init_vulkan()
{
	if (m_validationLayers && m_validationLayers->checkValidationLayersFailed())
		throw std::runtime_error("Validation layers requested, but not available!");

	VkSurfaceKHR surface;
	if (!vkinit::Instance::createInstance(m_instance, m_window.get(), m_validationLayers.get()) ||
		!vkinit::Surface::createSurface(surface, m_instance, m_window.get()))
		return false;

	m_presentationHardware = MAKEUNQ<Presentation::HardwareDevice>(m_instance, surface);
	m_presentationDevice = MAKEUNQ<Presentation::Device>(m_presentationHardware->getActiveGPU(), surface, m_window.get(), m_validationLayers.get());

	m_memoryAllocator = MAKEUNQ<VkMemoryAllocator>(m_instance, m_presentationHardware->getActiveGPU(), m_presentationDevice->getDevice());

	m_presentationTarget = MAKEUNQ<Presentation::PresentationTarget>(*m_presentationHardware, * m_presentationDevice, m_window.get(), true);
	m_framePresentation = MAKEUNQ<Presentation::FrameCollection>(m_presentationDevice->getDevice(), m_presentationDevice->getCommandPool());
	
	return m_presentationHardware->isInitialized() &&
		m_presentationDevice->isInitialized() &&

		m_memoryAllocator->isInitialized() &&

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
	
	CommandObjectsWrapper::renderIndexedMeshes(buffer, m_presentationTarget->getPipeline(), m_presentationTarget->getPipelineLayout(), m_presentationTarget->getRenderPass(),
		m_presentationTarget->getSwapchainFrameBuffers(imageIndex), m_presentationTarget->getSwapchainExtent(), *m_cam, m_openScene->getGraphicsMeshes(), descriptorSets, m_frameNumber);

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
		m_texture->release(m_presentationDevice->getDevice());

		m_imgui->release(m_presentationDevice->getDevice());

		m_openScene->release(m_memoryAllocator->m_allocator);

		m_framePresentation->releaseFrameResources();

		vkDestroyDescriptorPool(m_presentationDevice->getDevice(), descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_presentationDevice->getDevice(), descriptorSetLayout, nullptr);
		m_presentationTarget->releasePipeline(m_presentationDevice->getDevice());

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