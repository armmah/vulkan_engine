#pragma once
#include <vk_types.h>
#include "Presentation/PresentationDevice.h"
#include "Presentation/Frame.h"

class PresentationFrames : IRequireInitialization
{
	REF<PresentationDevice const> m_presentationDevice;
	std::vector<Frame> m_frameCollection;
	int m_currentFrameIndex = 0;
	bool m_fullyInitialized = false;

public:
	PresentationFrames(REF<PresentationDevice const> presentationDevice, int frameCount = 2);

	bool IRequireInitialization::isInitialized() const { return m_fullyInitialized; }

	Frame getNextFrame();
	Frame getNextFrameAndWaitOnFence();

	VkResult acquireImageFromSwapchain(uint32_t& imageIndex, VkSwapchainKHR swapchain);

	void releaseFrameResources();
};
