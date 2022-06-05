#pragma once
#include "vulkan/vulkan.h"
#include "Presentation/Device.h"

class ImGuiHandle
{
public:
	ImGuiHandle(VkInstance instance, VkPhysicalDevice activeGPU, const Presentation::Device* presentationDevice, VkRenderPass renderPass, uint32_t imageCount, SDL_Window* window);

	void draw(SDL_Window* window, Camera* cam);

	void release(VkDevice device);

private:
	const uint32_t DESC_POOL_SIZE = 100u;
	VkDescriptorPool m_descriptorPool;
};
