#pragma once
#include "pch.h"
#include "VkTypes/InitializersUtility.h"
#include "Engine/RenderLoopStatistics.h"

class ImGuiHandle;
class Scene;
class Camera;
class DescriptorPoolManager;
class Material;
class Window;
namespace Presentation
{
	class Device;
	class PresentationTarget;
	class FrameCollection;
	class HardwareDevice;
}

class VulkanEngine
{
public:
	VulkanEngine();
	~VulkanEngine();

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

	UNQ<DescriptorPoolManager> m_descriptorPoolManager;

	bool m_isInitialized { false };

	UNQ<Window> m_window;
	VkExtent2D m_startingWindowSize{ 800 , 600 };
	uint32_t m_frameNumber{ 0 };
	FrameStats m_renderLoopStatistics{};

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

