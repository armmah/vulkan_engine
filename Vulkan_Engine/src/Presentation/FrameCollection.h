#pragma once
#include "pch.h"
#include "Presentation/Device.h"
#include "Presentation/Frame.h"

namespace Presentation
{
	class FrameCollection : IRequireInitialization
	{
	public:
		FrameCollection(VkDevice device, VkCommandPool pool, uint32_t frameCount = SWAPCHAIN_IMAGE_COUNT);

		bool IRequireInitialization::isInitialized() const override { return m_fullyInitialized; }

		uint32_t getImageCount() { return as_uint32(m_frameCollection.size()); }

		Frame getNextFrame();
		Frame getNextFrameAndWaitOnFence();

		VkResult acquireImageFromSwapchain(uint32_t& imageIndex, VkSwapchainKHR m_swapchain);

		void releaseFrameResources();

	private:
		bool m_fullyInitialized = false;

		VkDevice m_device;
		std::vector<Frame> m_frameCollection;
		uint32_t m_currentFrameIndex = 0;
	};
}