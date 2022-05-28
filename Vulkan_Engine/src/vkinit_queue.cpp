#include "pch.h"
#include "vk_types.h"
#include <vector>

namespace vkinit
{
	bool Queue::satisfiesQueueRequirements(VkQueueFlags flags) { return flags & requiredFlags; }

	void Queue::findQueueFamilies(QueueFamilyIndices& indices, VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSurfSupport = false;
			if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSurfSupport) != VK_SUCCESS)
			{
				printf("Verbose-Warning: GetPhysicalDeviceSurfaceSupport failed for a device");
			}

			if (presentSurfSupport)
			{
				indices.presentFamily = i;
			}

			i++;
		}
	}

	std::vector<VkDeviceQueueCreateInfo> Queue::getQueueCreateInfo(QueueFamilyIndices indices, const float* queuePriority)
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos
		{
			deviceQueueCreateInfo(indices.graphicsFamily.value(), queuePriority),
			deviceQueueCreateInfo(indices.presentFamily.value(), queuePriority)
		};

		return queueCreateInfos;
	}

	VkDeviceQueueCreateInfo Queue::deviceQueueCreateInfo(uint32_t queueFamily, const float* queuePriority)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = queuePriority;

		return queueCreateInfo;
	}
}