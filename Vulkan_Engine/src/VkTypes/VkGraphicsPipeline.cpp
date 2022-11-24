#include "pch.h"
#include "VkGraphicsPipeline.h"

VkGraphicsPipeline& VkGraphicsPipeline::operator =(VkGraphicsPipeline other)
{
	std::swap(m_pipeline, other.m_pipeline);
	std::swap(m_pipelineLayout, other.m_pipelineLayout);
	return *this;
}