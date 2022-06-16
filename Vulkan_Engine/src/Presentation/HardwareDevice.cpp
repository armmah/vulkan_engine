#include "pch.h"
#include "HardwareDevice.h"
#include "VkTypes/InitializersUtility.h"
#include "VkTypes/vkInitQueueFamily.h"

namespace Presentation
{
	VkSurfaceFormatKHR HardwareDevice::chooseSwapSurfaceFormat() const
	{
		for (const auto& availableFormat : m_formats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return m_formats[0];
	}

	VkPresentModeKHR HardwareDevice::chooseSwapPresentMode() const
	{
		for (const auto& availablePresentMode : m_presentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	void HardwareDevice::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR>& formats, std::vector<VkPresentModeKHR>& presentModes) const
	{
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());
		}
	}

	int HardwareDevice::rateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface) const
	{
		int score = 0;

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		QueueFamilyIndices queueFamily;
		vkinit::Queue::findQueueFamilies(queueFamily, device, surface);
		if (!queueFamily.isInitialized())
		{
			printf("Physical device '%s' is discarded because it doesn't satisfy the QueueManager requirements.\n", deviceProperties.deviceName);
			return 0;
		}

		std::string unsupportedExtensions;
		if (!checkDeviceExtensionSupport(device, vkinit::Instance::requiredExtensions, unsupportedExtensions))
		{
			printf("Physical device '%s' is discarded because it doesn't support the required extensions: %s.\n", deviceProperties.deviceName, unsupportedExtensions.c_str());
			return 0;
		}

		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
		querySwapChainSupport(device, surface, formats, presentModes);
		if (formats.empty() || presentModes.empty())
		{
			printf("Physical device '%s' is discarded because the swap chain detail support is not adequete.\n", deviceProperties.deviceName);
			return 0;
		}

		if (supportedFeatures.samplerAnisotropy)
		{
			score += 5;
		}

		// Discrete GPUs have a significant performance advantage
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) 
		{
			score += 1000;
		}

		return score;
	}

	bool HardwareDevice::checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& requiredExtensions, std::string& unsupportedExtensionNames) const
	{
		uint32_t extCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, availableExtensions.data());

		std::set<std::string> required(requiredExtensions.begin(), requiredExtensions.end());
		for (const auto& ext : availableExtensions)
		{
			required.erase(ext.extensionName);
		}

		if (required.size() > 0)
		{
			unsupportedExtensionNames = "\n{\n";
			for (const auto& name : required)
			{
				unsupportedExtensionNames.append("  '" + name + "'\n");
			}
			unsupportedExtensionNames.append("}");
		}

		return required.empty();
	}

	bool HardwareDevice::pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
	{
		m_chosenGPU = VK_NULL_HANDLE;

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			printf("failed to find GPUs with Vulkan support!");
			return false;
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		int score = -1;
		VkPhysicalDevice bestDevice;

		for (const auto& device : devices)
		{
			auto newScore = rateDeviceSuitability(device, surface);
			if (newScore > score)
			{
				score = newScore;
				bestDevice = device;
			}
		}

		// Check if the best candidate is suitable at all
		if (score > 0)
		{
			m_chosenGPU = bestDevice;

			querySwapChainSupport(bestDevice, surface, m_formats, m_presentModes);

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(bestDevice, &deviceProperties);

			printf("Chosen the '%s' physical device.\n", deviceProperties.deviceName);
		}
		else
		{
			printf("failed to find a suitable GPU!");
			return false;
		}

		return m_chosenGPU != VK_NULL_HANDLE;
	}
}