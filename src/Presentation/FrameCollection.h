#pragma once
#include <vk_types.h>
#include "Presentation/Device.h"
#include "Presentation/Frame.h"

namespace Presentation
{
	class FrameCollection : IRequireInitialization
	{
		REF<Device const> m_presentationDevice;
		std::vector<Frame> m_frameCollection;
		int m_currentFrameIndex = 0;
		bool m_fullyInitialized = false;

	public:
		FrameCollection(REF<Device const> presentationDevice, int frameCount = 2);

		bool IRequireInitialization::isInitialized() const { return m_fullyInitialized; }

		Frame getNextFrame();
		Frame getNextFrameAndWaitOnFence();

		VkResult acquireImageFromSwapchain(uint32_t& imageIndex, VkSwapchainKHR swapchain);

		void releaseFrameResources();
	};
}