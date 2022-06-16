#pragma once
#include "pch.h"
#include "Interfaces/IRequireInitialization.h"

struct QueueFamilyIndices : IRequireInitialization
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IRequireInitialization::isInitialized() const override { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

namespace vkinit
{
	struct Queue
	{
		static constexpr VkQueueFlags requiredFlags = VK_QUEUE_GRAPHICS_BIT;

		constexpr bool satisfiesQueueRequirements(VkQueueFlags flags);

		static void findQueueFamilies(QueueFamilyIndices& indices, VkPhysicalDevice device, VkSurfaceKHR surface);
		static std::vector<VkDeviceQueueCreateInfo> getQueueCreateInfo(QueueFamilyIndices indices, const float* queuePriority);
		static VkDeviceQueueCreateInfo deviceQueueCreateInfo(uint32_t queueFamily, const float* queuePriority);
	};
}