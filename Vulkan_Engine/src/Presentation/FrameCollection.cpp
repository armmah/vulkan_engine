#include "pch.h"
#include "FrameCollection.h"

namespace Presentation
{
	FrameCollection::FrameCollection(VkDevice device, VkCommandPool pool, uint32_t frameCount) :
		m_device(device)
	{
		m_fullyInitialized = true;

		m_frameCollection.reserve(frameCount);
		for (uint32_t i = 0; i < frameCount; i++)
		{
			m_frameCollection.push_back(Frame(device, pool));
			m_fullyInitialized &= m_frameCollection[i].isInitialized();
		}
	}

	Frame FrameCollection::getNextFrame()
	{
		m_currentFrameIndex = (m_currentFrameIndex + 1) % m_frameCollection.size();
		return m_frameCollection[m_currentFrameIndex];
	}

	Frame FrameCollection::getNextFrameAndWaitOnFence()
	{
		auto frame = getNextFrame();
		frame.waitOnAcquireFence(m_device);
		return frame;
	}

	VkResult FrameCollection::acquireImageFromSwapchain(uint32_t& imageIndex, VkSwapchainKHR m_swapchain)
	{
		return vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_frameCollection[m_currentFrameIndex].getImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);
	}

	void FrameCollection::releaseFrameResources()
	{
		for (auto& frame : m_frameCollection)
		{
			frame.release(m_device);
		}

		m_frameCollection.clear();
		m_currentFrameIndex = 0;
		m_fullyInitialized = false;
	}
}