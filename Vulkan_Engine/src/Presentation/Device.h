#pragma once
#include <vk_types.h>
#include <functional>

namespace Presentation
{
	class Device : IRequireInitialization
	{
	public:
		Device(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const SDL_Window* windowPtr, const VulkanValidationLayers* validationLayers);

		bool IRequireInitialization::isInitialized() const override { return m_isInitialized; }

		VkDevice getDevice() const { return m_vkdevice; }
		VkSurfaceKHR getSurface() const { return m_surface; }
		VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
		VkQueue getPresentQueue() const { return m_presentQueue; }
		VkCommandPool getCommandPool() const { return m_commandPool; }
		const SDL_Window* getWindowRef() const { return m_window; }

		void submitImmediatelyAndWaitCompletion(const std::function<void(VkCommandBuffer cmd)>&& commandForExecution) const
		{
			auto cmdPool = getCommandPool();
			VkCommandBuffer cmdBuffer;
			vkinit::Commands::createSingleCommandBuffer(cmdBuffer, cmdPool, m_vkdevice);

			VkFence fence;
			vkinit::Synchronization::createFence(fence, m_vkdevice, false);

			{
				CommandObjectsWrapper::CommandBufferScope sc(cmdBuffer);
				commandForExecution(cmdBuffer);
			}

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBuffer;
			vkQueueSubmit(getGraphicsQueue(), 1, &submitInfo, fence);

			vkWaitForFences(m_vkdevice, 1, &fence, VK_TRUE, UINT64_MAX);
			vkDestroyFence(m_vkdevice, fence, nullptr);
		}

		void release();

	private:
		bool createCommandPool();

		bool createLogicalDevice(VkPhysicalDevice physicalDevice);

	private:
		bool m_isInitialized = false;

		VkDevice m_vkdevice;
		VkSurfaceKHR m_surface;
		QueueFamilyIndices m_queueIndices;

		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;

		VkCommandPool m_commandPool;

		const SDL_Window* m_window;
		const VulkanValidationLayers* m_validationLayers;
	};
}