#pragma once
#include "pch.h"
#include "Interfaces/IRequireInitialization.h"
#include "VkTypes/vkInitQueueFamily.h"

class VulkanValidationLayers;
class Window;

namespace Presentation
{
	class Device : IRequireInitialization
	{
	public:
		Device(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const Window* windowPtr, const VulkanValidationLayers* validationLayers);

		bool IRequireInitialization::isInitialized() const override { return m_isInitialized; }

		VkDevice getDevice() const { return m_vkdevice; }
		VkSurfaceKHR getSurface() const { return m_surface; }
		VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
		VkQueue getPresentQueue() const { return m_presentQueue; }
		VkCommandPool getCommandPool() const { return m_commandPool; }

		bool submitImmediatelyAndWaitCompletion(const std::function<void(VkCommandBuffer cmd)>&& commandForExecution) const;

		void release();

	private:
		bool m_isInitialized = false;

		VkDevice m_vkdevice;
		VkSurfaceKHR m_surface;
		QueueFamilyIndices m_queueIndices;

		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;

		VkCommandPool m_commandPool;

		const Window* m_window;
		const VulkanValidationLayers* m_validationLayers;

		bool createCommandPool();
		bool createLogicalDevice(VkPhysicalDevice physicalDevice);
	};
}