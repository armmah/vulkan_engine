// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "vk_types.h"

#include "Presentation/HardwareDevice.h"
#include "Presentation/Device.h"
#include "Presentation/PresentationTarget.h"
#include "Presentation/Frame.h"
#include "Presentation/FrameCollection.h"

#include <vector>
#include <optional>
#include <string>

#include "EngineCore/Scene.h"

class VulkanEngine
{
public:

	std::string applicationName = "Hello Triangle";

	VkInstance m_instance; // Vulkan library handle

	UNQ<VulkanValidationLayers> m_validationLayers;
	VmaAllocator m_memoryAllocator;
	//VkDebugUtilsMessengerEXT _debug_messenger; // Vulkan debug output handle

	UNQ<Presentation::HardwareDevice> m_presentationHardware;
	UNQ<Presentation::Device> m_presentationDevice;
	UNQ<Presentation::PresentationTarget> m_presentationTarget;
	UNQ<Presentation::FrameCollection> m_framePresentation;

	VkDescriptorPool m_imguiPool;
	UNQ<Scene> m_openScene;
	UNQ<Camera> m_cam;

	bool _isInitialized { false };

	VkExtent2D m_startingWindowSize{ 800 , 600 };
	uint32_t m_frameNumber{ 0 };
	struct SDL_Window* m_window{ nullptr };


	//initializes everything in the engine
	void init(bool requestValidationLayers);

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	//run main loop
	void run();


private:

	void initializeTheWindow();

	void initImGui();

	bool init_vulkan(SDL_Window* window);
	
	bool handleFailedToAcquireImageIfNecessary(VkResult imageAcquireResult);
};

