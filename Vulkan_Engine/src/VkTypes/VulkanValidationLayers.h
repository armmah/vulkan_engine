#pragma once
#include "pch.h"

class VulkanValidationLayers
{
	const char* m_validationLayerName = "VK_LAYER_KHRONOS_validation";

	bool checkValidationLayerSupport();

public:
	bool checkValidationLayersFailed();

	void applyValidationLayers(VkInstanceCreateInfo& createInfo) const;
	void applyValidationLayers(VkDeviceCreateInfo& createInfo) const;
};