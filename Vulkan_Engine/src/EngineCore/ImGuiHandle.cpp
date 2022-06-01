#include "pch.h"
#include "ImGuiHandle.h"

ImGuiHandle::ImGuiHandle(VkInstance instance, VkPhysicalDevice activeGPU, const Presentation::Device* presentationDevice, VkRenderPass renderPass, uint32_t imageCount, SDL_Window* window)
{
	auto device = presentationDevice->getDevice();

	// 1: create descriptor pool for IMGUI
	VkDescriptorPoolSize pool_sizes[] =
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
	ImGui_ImplSDL2_InitForVulkan(window);

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

	{
		auto cmdPool = presentationDevice->getCommandPool();
		VkCommandBuffer cmdBuffer;
		vkinit::Commands::createSingleCommandBuffer(cmdBuffer, cmdPool, device);

		VkFence fence;
		vkinit::Synchronization::createFence(fence, device, false);

		//execute a gpu command to upload imgui font textures
		{
			CommandObjectsWrapper::CommandBufferScope sc(cmdBuffer);
			ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;
		vkQueueSubmit(presentationDevice->getGraphicsQueue(), 1, &submitInfo, fence);

		vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
		vkDestroyFence(device, fence, nullptr);
	}

	//clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImGuiHandle::release(VkDevice device)
{
	//add the destroy the imgui created structures
	vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
	ImGui_ImplVulkan_Shutdown();
}
