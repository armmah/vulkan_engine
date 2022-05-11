#include "PresentationFrames.h"

PresentationFrames::PresentationFrames(REF<PresentationDevice const> presentationDevice, int frameCount) :
	m_presentationDevice(presentationDevice)
{
	m_fullyInitialized = true;

	auto device = presentationDevice->getDevice();
	auto pool = presentationDevice->getCommandPool();

	m_frameCollection.reserve(frameCount);
	for (int i = 0; i < frameCount; i++)
	{
		m_frameCollection.push_back(Frame(device, pool));
		m_fullyInitialized &= m_frameCollection[i].isInitialized();
	}
}

Frame PresentationFrames::getNextFrame()
{
	m_currentFrameIndex = (m_currentFrameIndex + 1) % m_frameCollection.size();
	return m_frameCollection[m_currentFrameIndex];
}

Frame PresentationFrames::getNextFrameAndWaitOnFence()
{
	auto frame = getNextFrame();
	frame.waitOnAcquireFence(m_presentationDevice->getDevice());
	return frame;
}

VkResult PresentationFrames::acquireImageFromSwapchain(uint32_t& imageIndex, VkSwapchainKHR swapchain)
{
	return vkAcquireNextImageKHR(m_presentationDevice->getDevice(), swapchain, UINT64_MAX, m_frameCollection[m_currentFrameIndex].getImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);
}

void PresentationFrames::releaseFrameResources()
{
	for (auto& frame : m_frameCollection)
	{
		frame.release(m_presentationDevice->getDevice());
	}

	m_frameCollection.clear();
	m_currentFrameIndex = 0;
	m_fullyInitialized = false;
}
