#pragma once
#include "pch.h"
#include "ShaderSource.h"

struct Shader
{
	VkDevice device;
	VkShaderModule vertShader,
		fragShader;

	Shader(VkDevice device, const ShaderSource& source)
		: device(device)
	{
		if (!createShaderModule(vertShader, source.getVertexSource(), device))
		{
			printf("Failed to compile the shader '%s'.", source.fragmentPath.c_str());
		}

		if (!createShaderModule(fragShader, source.getFragmentSource(), device))
		{
			printf("Failed to compile the shader '%s'.", source.fragmentPath.c_str());
		}
	}

	static bool createShaderModule(VkShaderModule& module, const std::vector<char>& code, VkDevice device)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		return vkCreateShaderModule(device, &createInfo, nullptr, &module) == VK_SUCCESS;
	}

	void release()
	{
		vkDestroyShaderModule(device, vertShader, nullptr);
		vkDestroyShaderModule(device, fragShader, nullptr);
	}
};