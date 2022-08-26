#pragma once
#include "pch.h"

struct FrameStats
{
	size_t pipelineCount;
	size_t descriptorSetCount;
	size_t drawCallCount;

	size_t frameNumber;
	int64_t renderLoop_ms;
};