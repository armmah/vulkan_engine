#pragma once
#include <vk_types.h>
#include "Presentation/Device.h"
#include "Presentation/Frame.h"

namespace Presentation
{
	class FrameCollection : IRequireInitialization
	{
	public:
		FrameCollection(REF<Device const> presentationDevice, int frameCount = 2);

		bool IRequireInitialization::isInitialized() const { return m_fullyInitialized; }

		Frame getNextFrame();
		Frame getNextFrameAndWaitOnFence();

		VkResult acquireImageFromSwapchain(uint32_t& imageIndex, VkSwapchainKHR m_swapchain);

		void releaseFrameResources();

	private:
		bool m_fullyInitialized = false;

		REF<Device const> m_presentationDevice;
		std::vector<Frame> m_frameCollection;
		int m_currentFrameIndex = 0;
	};
}