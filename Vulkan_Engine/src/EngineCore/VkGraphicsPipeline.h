#pragma once
#include "pch.h"

struct VkGraphicsPipeline
{
	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;

	VkGraphicsPipeline& operator =(VkGraphicsPipeline other);
};
