#pragma once
#include "pch.h"

struct ShaderSource;

struct Shader
{
	VkShaderModule vertShader,
		fragShader;

	Shader(VkDevice device, const ShaderSource& source);
	static bool createShaderModule(VkShaderModule& module, const std::vector<char>& code, VkDevice device);

	void release(VkDevice device);
};