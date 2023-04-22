#pragma once
#include "pch.h"
#include "Interfaces/IRequireInitialization.h"

namespace Presentation
{
	class Frame : IRequireInitialization
	{
	public:
		Frame(VkDevice device, VkCommandPool pool)
		{
			m_isInitialized = initialize(device, pool);
		}

		bool IRequireInitialization::isInitialized() const override { return m_isInitialized; }

		VkCommandBuffer getCommandBuffer() const { return m_buffer; }
		VkSemaphore getImageAvailableSemaphore() const { return m_imageAvailableSemaphore; }
		VkSemaphore getRenderFinishedSemaphore() const { return m_renderFinishedSemaphore; }
		VkFence getInFlightFence() const { return m_inFlightFence; }

		void submitToQueue(VkQueue graphicsQueue);

		void present(uint32_t imageIndex, VkSwapchainKHR swapChain, VkQueue presentQueue);

		void waitOnAcquireFence(VkDevice device);
		void resetAcquireFence(VkDevice device);

		void release(VkDevice device);

	private:
		bool m_isInitialized = false;

		VkCommandBuffer m_buffer;

		VkSemaphore m_imageAvailableSemaphore;
		VkSemaphore m_renderFinishedSemaphore;
		VkFence m_inFlightFence;

		bool initialize(VkDevice device, VkCommandPool pool);
	};
}