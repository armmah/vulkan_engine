#pragma once
#include "pch.h"

struct FrameStats
{
	uint32_t pipelineCount;
	uint32_t descriptorSetCount;
	uint32_t drawCallCount;

	uint32_t frameNumber;
	int64_t renderLoop_ms;
};