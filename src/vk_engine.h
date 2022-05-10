// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "vk_types.h"
#include <vector>
#include <optional>
#include <string>

class VulkanEngine
{
public:

	std::string applicationName = "Hello Triangle";

	VkInstance m_instance; // Vulkan library handle
	VulkanValidationLayers m_validationLayers;
	//VkDebugUtilsMessengerEXT _debug_messenger; // Vulkan debug output handle

	std::shared_ptr<HardwarePresentationDevice> m_presentationHardware;
	std::shared_ptr<PresentationDevice> m_presentationDevice;
	std::unique_ptr<PresentationTarget> m_swapchain;
	std::unique_ptr<PresentationFrames> m_framePresentation;

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

