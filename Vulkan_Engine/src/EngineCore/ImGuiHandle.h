#pragma once
#include "pch.h"
#include "vulkan/vulkan.h"
#include "Presentation/Device.h"

class Camera;
class Window;

class ImGuiHandle
{
public:
	ImGuiHandle(VkInstance instance, VkPhysicalDevice activeGPU, const Presentation::Device* presentationDevice, VkRenderPass renderPass, uint32_t imageCount, Window* window);

	void draw(Camera* cam);

	void release(VkDevice device);

private:
	const uint32_t DESC_POOL_SIZE = 100u;
	VkDescriptorPool m_descriptorPool;

	const Window* m_window;
};
