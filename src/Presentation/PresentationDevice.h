#pragma once
#include <vk_types.h>

class PresentationDevice : IRequireInitialization
{
	VkDevice m_vkdevice;
	VkSurfaceKHR m_surface;
	QueueFamilyIndices m_queueIndices;

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	VkCommandPool m_commandPool;

	const SDL_Window* m_window;
	std::optional<VulkanValidationLayers> m_validationLayers;
	bool m_isInitialized = false;

public:
	PresentationDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const SDL_Window* windowPtr, std::optional<VulkanValidationLayers> validationLayers);

	bool IRequireInitialization::isInitialized() const { return m_isInitialized; }

	VkDevice getDevice() const { return m_vkdevice; }
	VkSurfaceKHR getSurface() const { return m_surface; }
	VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
	VkQueue getPresentQueue() const { return m_presentQueue; }
	VkCommandPool getCommandPool() const { return m_commandPool; }
	const SDL_Window* getWindowRef() const { return m_window; }

	void release();

private:
	bool createCommandPool();

	bool createLogicalDevice(VkPhysicalDevice physicalDevice);

};