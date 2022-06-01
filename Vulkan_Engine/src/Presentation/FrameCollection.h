#pragma once
#include <vk_types.h>
#include "Presentation/Device.h"
#include "Presentation/Frame.h"

namespace Presentation
{
	class FrameCollection : IRequireInitialization
	{
	public:
		FrameCollection(VkDevice device, VkCommandPool pool, uint32_t frameCount = 2);

		bool IRequireInitialization::isInitialized() const { return m_fullyInitialized; }

		uint32_t getImageCount() { return as_uint32(m_frameCollection.size()); }

		Frame getNextFrame();
		Frame getNextFrameAndWaitOnFence();

		VkResult acquireImageFromSwapchain(uint32_t& imageIndex, VkSwapchainKHR m_swapchain);

		void releaseFrameResources();

	private:
		bool m_fullyInitialized = false;

		VkDevice m_device;
		std::vector<Frame> m_frameCollection;
		int m_currentFrameIndex = 0;
	};
}