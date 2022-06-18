#include "pch.h"
#include "Engine/Window.h"
#include "InitializersUtility.h"

bool vkinit::Surface::createSurface(VkSurfaceKHR& surface, VkInstance instance, Window* window)
{
	return SDL_Vulkan_CreateSurface(window->get(), instance, &surface);
}
