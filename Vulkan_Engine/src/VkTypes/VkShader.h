#pragma once
#include "pch.h"

struct ShaderSource;

struct Shader
{
	VkShaderModule vertShader,
		fragShader;

	Shader(VkDevice device, const ShaderSource& source);
	static bool createShaderModule(VkShaderModule& module, const std::vector<char>& code, VkDevice device);

	static void ensureDefaultShader(VkDevice device);
	//static bool findShader(std::string shaderName);
	static const Shader* findShader(uint32_t identifier) { return identifier < globalShaderList.size() ? globalShaderList[identifier].get() : nullptr; }

	void release(VkDevice device);
	static void releaseGlobalShaderList(VkDevice device);

private:
	inline static std::vector<std::unique_ptr<Shader>> globalShaderList;
};