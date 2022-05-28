#include <pch.h>
#include "Device.h"

namespace Presentation
{
	Device::Device(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const SDL_Window* windowPtr, const VulkanValidationLayers* validationLayers)
		: m_surface(surface), m_window(windowPtr), m_validationLayers(validationLayers)
	{
		m_isInitialized =
			createLogicalDevice(physicalDevice) &&
			createCommandPool();
	}

	void Device::release()
	{
		vkDestroyCommandPool(m_vkdevice, m_commandPool, nullptr);
		vkDestroyDevice(m_vkdevice, nullptr);
	}

	bool Device::createCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_queueIndices.graphicsFamily.value();

		return vkCreateCommandPool(m_vkdevice, &poolInfo, nullptr, &m_commandPool) == VK_SUCCESS;
	}

	bool Device::createLogicalDevice(VkPhysicalDevice physicalDevice)
	{
		VkPhysicalDeviceFeatures deviceFeatures{};

		vkinit::Queue::findQueueFamilies(m_queueIndices, physicalDevice, m_surface);
		float queuePriority = 1.0;
		auto queueCreateInfos = vkinit::Queue::getQueueCreateInfo(m_queueIndices, &queuePriority);

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		auto& requiredExtensions = vkinit::Instance::requiredExtensions;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		if (m_validationLayers)
		{
			m_validationLayers->applyValidationLayers(createInfo);
		}

		bool isSuccess = vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_vkdevice) == VK_SUCCESS;
		if (isSuccess)
		{
			vkGetDeviceQueue(m_vkdevice, m_queueIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
			vkGetDeviceQueue(m_vkdevice, m_queueIndices.presentFamily.value(), 0, &m_presentQueue);
		}

		return isSuccess;
	}
}