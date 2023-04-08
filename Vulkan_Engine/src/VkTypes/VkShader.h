#pragma once
#include "pch.h"
#include "Common.h"

struct ShaderSource;

struct VkShader
{
	VkShaderModule vertShader,
		fragShader;

	VkShader(VkDevice device, const ShaderSource& source);
	static bool createShaderModule(VkShaderModule& module, const std::vector<char>& code, VkDevice device);

	static void ensureDefaultShader(VkDevice device);

	//static bool findShader(std::string shaderName);
	static const VkShader* findShader(uint32_t identifier) { return identifier < globalShaderList.size() ? globalShaderList[identifier].get() : nullptr; }
	static const VkShader* createGlobalShader(VkDevice device, const ShaderSource& source) 
	{
		insertShaderToGlobalList
		(
			device, VkShader(device, source)
		);

		return globalShaderList.back().get();
	}

	void release(VkDevice device);
	static void releaseGlobalShaderList(VkDevice device);

private:
	inline static std::vector<std::unique_ptr<VkShader>> globalShaderList;

	static void insertShaderToGlobalList(VkDevice device, VkShader shader)
	{
		globalShaderList.push_back(MAKEUNQ<VkShader>(std::move(shader)));
	}
};