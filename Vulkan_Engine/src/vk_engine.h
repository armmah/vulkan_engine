#pragma once
#include "pch.h"

#include "Presentation/HardwareDevice.h"
#include "Presentation/Device.h"
#include "Presentation/PresentationTarget.h"
#include "Presentation/Frame.h"
#include "Presentation/FrameCollection.h"

#include "EngineCore/Scene.h"
#include "Camera.h"
#include "VkTypes/VkTexture.h"
#include "EngineCore/ImGuiHandle.h"
#include "Texture.h"
#include "VkTypes/VkMemoryAllocator.h"
#include "Engine/Window.h"

class VulkanEngine
{
public:

	std::string m_applicationName = "Basic VK Engine";

	VkInstance m_instance{}; // Vulkan library handle
	UNQ<VkMemoryAllocator> m_memoryAllocator;

	UNQ<VulkanValidationLayers> m_validationLayers;
	//VkDebugUtilsMessengerEXT _debug_messenger; // Vulkan debug output handle

	UNQ<Presentation::HardwareDevice> m_presentationHardware;
	UNQ<Presentation::Device> m_presentationDevice;
	UNQ<Presentation::PresentationTarget> m_presentationTarget;
	UNQ<Presentation::FrameCollection> m_framePresentation;

	UNQ<ImGuiHandle> m_imgui;

	UNQ<Scene> m_openScene;
	UNQ<Camera> m_cam;
	UNQ<VkTexture2D> m_texture;
	VkDescriptorPool descriptorPool{};
	VkDescriptorSetLayout descriptorSetLayout{};
	std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT> descriptorSets;

	bool m_isInitialized { false };

	UNQ<Window> m_window;
	VkExtent2D m_startingWindowSize{ 800 , 600 };
	uint32_t m_frameNumber{ 0 };


	//initializes everything in the engine
	void init(bool requestValidationLayers);

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	//run main loop
	void run();


private:

	bool init_vulkan();
	
	bool handleFailedToAcquireImageIfNecessary(VkResult imageAcquireResult);
};

