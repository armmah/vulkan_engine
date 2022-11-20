#include "pch.h"
#include "ImGuiHandle.h"
#include "Common.h"
#include "Camera.h"
#include "Engine/Window.h"
#include "Presentation/Device.h"
#include "Engine/RenderLoopStatistics.h"

ImGuiHandle::ImGuiHandle(VkInstance instance, VkPhysicalDevice activeGPU, const Presentation::Device* presentationDevice, VkRenderPass renderPass, uint32_t imageCount, Window* window)
	: m_window(window)
{
	auto device = presentationDevice->getDevice();

	// 1: create descriptor pool for IMGUI
	const VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, DESC_POOL_SIZE },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, DESC_POOL_SIZE }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = DESC_POOL_SIZE;
	pool_info.poolSizeCount = as_uint32(std::size(pool_sizes));
	pool_info.pPoolSizes = pool_sizes;

	vkCreateDescriptorPool(device, &pool_info, nullptr, &m_descriptorPool);

	// 2: initialize imgui library

	//this initializes the core structures of imgui
	ImGui::CreateContext();

	//this initializes imgui for SDL
	ImGui_ImplSDL2_InitForVulkan(window->get());

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance;
	init_info.PhysicalDevice = activeGPU;
	init_info.Device = device;
	init_info.Queue = presentationDevice->getGraphicsQueue();
	init_info.DescriptorPool = m_descriptorPool;
	init_info.MinImageCount = 2;
	init_info.ImageCount = imageCount;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, renderPass);

	presentationDevice->submitImmediatelyAndWaitCompletion([=](VkCommandBuffer cmd)
	{
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
	});

	//clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImGuiHandle::draw(FrameStats stats, Camera* cam, DirectionalLightParams* light, FrameSettings* settings)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame(m_window->get());

	ImGui::NewFrame();

	ImGui::Begin("Menu");
	{
		std::string statsText =
			"Draw Calls: " + std::to_string(stats.drawCallCount) +
			"\nRenderLoop: " + std::to_string(stats.renderLoop_ms) + " ms"
			+ "\n\nPipeline count: " + std::to_string(stats.pipelineCount) +
			+"\nDescriptor set count: " + std::to_string(stats.descriptorSetCount);
		ImGui::Text(statsText.c_str());
	}

	bool frameSettingsCollapsed = ImGui::CollapsingHeader("Settings");
	if (frameSettingsCollapsed)
	{
		ImGui::Checkbox("Shadows", &settings->enableShadowPass);
		// ImGui::Checkbox("Forward", &settings->enableForwardPass);
		ImGui::Checkbox("Debug", &settings->enableDebugShadowMap);
	}

	bool cameraParamsCollapsed = ImGui::CollapsingHeader("Camera params");
	if (cameraParamsCollapsed)
	{
		auto speedMul = cam->getSpeedMultiplier();
		ImGui::SliderFloat("Speed", &speedMul, 0.01f, 500.0f);
		if (speedMul != cam->getSpeedMultiplier())
			cam->setSpeedMultiplier(speedMul);

		auto pos = glm::vec3(cam->getPosition());
		ImGui::InputFloat3("Position", &pos[0]);

		auto yaw_pitch = glm::vec2(cam->getYaw(), cam->getPitch());
		ImGui::InputFloat2("Yaw / Pitch", &yaw_pitch[0]);
	}

	bool isLightCollapsed = ImGui::CollapsingHeader("Light");
	if (isLightCollapsed)
	{
		ImGui::DragFloat3("Light Pitch / Yaw / Distance", &light->pitch);
		ImGui::DragFloat("Depth Bias", &light->depthBias, 0.025f, -3.f, 3.f);
		ImGui::DragFloat("Normal Bias", &light->normalBias, 0.025f, -1.f, 1.f);
		ImGui::DragFloat("Ambient light", &light->ambient, 0.02f, 0.0f, 0.5f);
	}
	ImGui::End();
	
	ImGui::Render();
}

void ImGuiHandle::release(VkDevice device)
{
	//add the destroy the imgui created structures
	vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
	ImGui_ImplVulkan_Shutdown();
}
