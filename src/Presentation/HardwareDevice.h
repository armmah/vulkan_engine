#pragma once
#include <vk_types.h>

namespace Presentation
{
	class HardwareDevice : IRequireInitialization
	{
		VkPhysicalDevice m_chosenGPU; // GPU chosen as the default device
		std::vector<VkSurfaceFormatKHR> m_formats;
		std::vector<VkPresentModeKHR> m_presentModes;

		bool m_isInitialized = false;

	public:
		HardwareDevice(VkInstance instance, VkSurfaceKHR surface)
		{
			m_isInitialized = pickPhysicalDevice(instance, surface);
		}

		bool IRequireInitialization::isInitialized() const { return m_isInitialized; }

		VkPhysicalDevice getActiveGPU() const { return m_chosenGPU; }

		VkSurfaceFormatKHR chooseSwapSurfaceFormat() const;
		VkPresentModeKHR chooseSwapPresentMode() const;

	private:
		void querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR>& formats, std::vector<VkPresentModeKHR>& presentModes) const;

		int rateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface) const;

		bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> requiredExtensions, std::string& unsupportedExtensionNames) const;

		bool pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
	};
}