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
	VulkanValidationLayers m_validationLayers;
	//VkDebugUtilsMessengerEXT _debug_messenger; // Vulkan debug output handle

	std::shared_ptr<Presentation::HardwareDevice> m_presentationHardware;
	std::shared_ptr<Presentation::Device> m_presentationDevice;
	std::unique_ptr<Presentation::PresentationTarget> m_presentationTarget;
	std::unique_ptr<Presentation::FrameCollection> m_framePresentation;

	VmaAllocator m_memoryAllocator;
	
	std::unique_ptr<Scene> m_openScene;

	bool _isInitialized { false };

	VkExtent2D m_startingWindowSize{ 800 , 600 };
	struct SDL_Window* m_window{ nullptr };

	//initializes everything in the engine
	void init();

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	//run main loop
	void run();


private:

	bool init_vulkan(SDL_Window* window);
	
	bool handleFailedToAcquireImageIfNecessary(VkResult imageAcquireResult);
};

