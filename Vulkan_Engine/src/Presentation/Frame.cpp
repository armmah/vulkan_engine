#include "pch.h"
#include "Frame.h"
#include "VkTypes/InitializersUtility.h"

namespace Presentation
{
	bool Frame::initialize(VkDevice device, VkCommandPool pool)
	{
		bool fullyInitialized = true;
		fullyInitialized &= vkinit::Synchronization::createSemaphore(m_imageAvailableSemaphore, device);
		fullyInitialized &= vkinit::Synchronization::createSemaphore(m_renderFinishedSemaphore, device);

		fullyInitialized &= vkinit::Synchronization::createFence(m_inFlightFence, device);

		fullyInitialized &= vkinit::Commands::createSingleCommandBuffer(m_buffer, pool, device);
		return fullyInitialized;
	}

	void Frame::submitToQueue(VkQueue graphicsQueue)
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_renderFinishedSemaphore;

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_imageAvailableSemaphore;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_buffer;

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_inFlightFence) != VK_SUCCESS)
			throw std::runtime_error("Failed to submit draw command buffer!");
	}

	void Frame::present(uint32_t imageIndex, VkSwapchainKHR swapChain, VkQueue presentQueue)
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &m_renderFinishedSemaphore;

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain;
		presentInfo.pImageIndices = &imageIndex;

		presentInfo.pResults = nullptr; // Optional

		vkQueuePresentKHR(presentQueue, &presentInfo);
	}

	void Frame::waitOnAcquireFence(VkDevice device)
	{
		vkWaitForFences(device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
	}

	void Frame::resetAcquireFence(VkDevice device)
	{
		vkResetFences(device, 1, &m_inFlightFence);
	}

	void Frame::release(VkDevice device)
	{
		vkDestroySemaphore(device, m_imageAvailableSemaphore, nullptr);
		vkDestroySemaphore(device, m_renderFinishedSemaphore, nullptr);
		vkDestroyFence(device, m_inFlightFence, nullptr);
	}
}