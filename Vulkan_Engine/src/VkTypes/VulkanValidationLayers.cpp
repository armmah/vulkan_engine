#include "pch.h"
#include "VulkanValidationLayers.h"

bool VulkanValidationLayers::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const auto& layerProperties : availableLayers)
	{
		if (strcmp(m_validationLayerName, layerProperties.layerName) == 0)
		{
			return true;
		}
	}

	return false;
}

bool VulkanValidationLayers::checkValidationLayersFailed() { return !checkValidationLayerSupport(); }

void VulkanValidationLayers::applyValidationLayers(VkInstanceCreateInfo& createInfo) const
{
	createInfo.enabledLayerCount = 1u;
	createInfo.ppEnabledLayerNames = &m_validationLayerName;

	printf("	=====[Validation layers enabled]=====\n");
}

void VulkanValidationLayers::applyValidationLayers(VkDeviceCreateInfo& createInfo) const
{
	createInfo.enabledLayerCount = 1u;
	createInfo.ppEnabledLayerNames = &m_validationLayerName;
}
