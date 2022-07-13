#include "pch.h"
#include "VkShader.h"
#include "ShaderSource.h"

Shader::Shader(VkDevice device, const ShaderSource& source)
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

bool Shader::createShaderModule(VkShaderModule& module, const std::vector<char>& code, VkDevice device)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	return vkCreateShaderModule(device, &createInfo, nullptr, &module) == VK_SUCCESS;
}

void Shader::release(VkDevice device)
{
	vkDestroyShaderModule(device, vertShader, nullptr);
	vkDestroyShaderModule(device, fragShader, nullptr);
}
