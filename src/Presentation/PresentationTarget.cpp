#include "PresentationTarget.h"

namespace Presentation
{
	bool PresentationTarget::createGraphicsPipeline(VkDevice device, VkCullModeFlagBits faceCullingMode = VK_CULL_MODE_BACK_BIT)
	{
		return createGraphicsPipeline(device, swapChainExtent, faceCullingMode);
	}
}