#pragma once
#include <vk_types.h>

class Frame : IRequireInitialization
{
	VkCommandBuffer m_buffer;

	VkSemaphore m_imageAvailableSemaphore;
	VkSemaphore m_renderFinishedSemaphore;
	VkFence m_inFlightFence;

	bool m_isInitialized = false;

	bool initialize(VkDevice device, VkCommandPool pool);

public:
	Frame(VkDevice device, VkCommandPool pool)
	{
		m_isInitialized = initialize(device, pool);
	}

	bool IRequireInitialization::isInitialized() const { return m_isInitialized; }

	VkCommandBuffer getCommandBuffer() const { return m_buffer; }
	VkSemaphore getImageAvailableSemaphore() const { return m_imageAvailableSemaphore; }
	VkSemaphore getRenderFinishedSemaphore() const { return m_renderFinishedSemaphore; }
	VkFence getInFlightFence() const { return m_inFlightFence; }

	void submitToQueue(VkQueue graphicsQueue);

	void present(uint32_t imageIndex, VkSwapchainKHR swapChain, VkQueue presentQueue);

	void waitOnAcquireFence(VkDevice device);
	void resetAcquireFence(VkDevice device);

	void release(VkDevice device);
};

