#pragma once
#include "pch.h"
#include "Interfaces/IRequireInitialization.h"

namespace Presentation
{
	class HardwareDevice : IRequireInitialization
	{
	public:
		HardwareDevice(VkInstance instance, VkSurfaceKHR surface)
		{
			m_isInitialized = pickPhysicalDevice(instance, surface);
		}

		bool IRequireInitialization::isInitialized() const override { return m_isInitialized; }

		VkPhysicalDevice getActiveGPU() const { return m_chosenGPU; }

		VkSurfaceFormatKHR chooseSwapSurfaceFormat() const;
		VkPresentModeKHR chooseSwapPresentMode() const;

	private:
		bool m_isInitialized = false;

		VkPhysicalDevice m_chosenGPU; // GPU chosen as the default device
		std::vector<VkSurfaceFormatKHR> m_formats;
		std::vector<VkPresentModeKHR> m_presentModes;

		void querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR>& formats, std::vector<VkPresentModeKHR>& presentModes) const;

		int rateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface) const;

		bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& requiredExtensions, std::string& unsupportedExtensionNames) const;

		bool pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
	};
}