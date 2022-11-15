#pragma once
#include "pch.h"

struct DirectionalLightParams
{
	float pitch, yaw, distance;
	float depthBias, normalBias, ambient, emptyPlaceholder;

	DirectionalLightParams(float pitch = 76.0f, float yaw = 90.0f, float distance = 30.f,
		float depthBias = 0.001f, float normalBias = -0.001f, float ambient = 0.01) : pitch(pitch), yaw(yaw), distance(distance), 
		depthBias(depthBias), normalBias(normalBias), ambient(ambient), emptyPlaceholder() { }

	const glm::vec4 getBiasAmbient() { return glm::vec4(depthBias, normalBias, ambient, 0.0f); }
};

struct FrameSettings
{
	bool enableShadowPass;
	bool enableForwardPass;

	bool enableDebugShadowMap;

	FrameSettings(bool forwardPass = true, bool shadowPass = true, bool debugShadowMap = false)
		: enableShadowPass(shadowPass), enableForwardPass(forwardPass), enableDebugShadowMap(debugShadowMap) { }
};

struct FrameStats
{
	size_t pipelineCount;
	size_t descriptorSetCount;
	size_t drawCallCount;

	size_t frameNumber;
	int64_t renderLoop_ms;
};