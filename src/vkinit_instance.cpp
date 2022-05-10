﻿#include <vk_types.h>

namespace vkinit
{
	const std::vector<const char*> Instance::requiredExtensions = {
		   VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	bool Instance::getRequiredExtensionsForPlatform(SDL_Window* window, unsigned int* extCount, const char** extensionNames)
	{
		return SDL_Vulkan_GetInstanceExtensions(window, extCount, extensionNames);
	}

	bool Instance::createInstance(VkInstance& instance, std::string applicationName, std::vector<const char*> extNames, std::optional<VulkanValidationLayers> validationLayers)
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = VK_API_VERSION_1_1;
		appInfo.pApplicationName = applicationName.c_str();

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extNames.size());
		createInfo.ppEnabledExtensionNames = extNames.data();

		if (validationLayers.has_value())
		{
			validationLayers.value().applyValidationLayers(createInfo);
		}

		return vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS;
	}
}