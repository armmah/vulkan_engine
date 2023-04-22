#pragma once
#include "pch.h"
#include "vulkan/vulkan.h"

class Camera;
class Window;
struct FrameStats;
struct FrameSettings;
struct DirectionalLightParams;
namespace Presentation
{
	class Device;
}

class ImGuiHandle
{
public:
	ImGuiHandle(VkInstance instance, VkPhysicalDevice activeGPU, const Presentation::Device* presentationDevice, VkRenderPass renderPass, uint32_t imageCount, Window* window);

	void draw(const FrameStats& stats, Camera* cam, DirectionalLightParams* light, FrameSettings* settings);

	void release(VkDevice device);

private:
	const uint32_t DESC_POOL_SIZE = 100u;
	VkDescriptorPool m_descriptorPool;

	const Window* m_window;
};
