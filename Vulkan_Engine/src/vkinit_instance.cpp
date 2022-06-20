#include "pch.h"
#include "Engine/Window.h"
#include "VkTypes/InitializersUtility.h"
#include "VkTypes/VulkanValidationLayers.h"

namespace vkinit
{
	const std::vector<const char*> Instance::requiredExtensions = {
		   VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	bool Instance::getRequiredExtensionsForPlatform(Window const* window, unsigned int* extCount, const char** extensionNames)
	{
		return SDL_Vulkan_GetInstanceExtensions(window->get(), extCount, extensionNames);
	}

	bool Instance::createInstance(VkInstance& instance, const char* applicationName, std::vector<const char*> extNames, const VulkanValidationLayers* validationLayers)
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = VK_API_VERSION_1_1;
		appInfo.pApplicationName = applicationName;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extNames.size());
		createInfo.ppEnabledExtensionNames = extNames.data();

		if (validationLayers)
		{
			validationLayers->applyValidationLayers(createInfo);
		}

		return vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS;
	}

	bool Instance::createInstance(VkInstance& instance, const Window* window, const VulkanValidationLayers* validationLayers)
	{
		unsigned int extCount = 0;
		if (!vkinit::Instance::getRequiredExtensionsForPlatform(window, &extCount, nullptr))
			return false;

		// SDL requires { "VK_KHR_surface", "VK_KHR_win32_surface" } extensions for windows
		std::vector<const char*> extensions(extCount);
		if (!vkinit::Instance::getRequiredExtensionsForPlatform(window, &extCount, extensions.data()))
			return false;

		return vkinit::Instance::createInstance(instance, window->getAppName().c_str(), extensions, validationLayers);
	}
}