#include "pch.h"
#include "Common.h"
#include "VkShader.h"
#include "ShaderSource.h"

VkShader::VkShader(VkDevice device, const ShaderSource& source)
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

bool VkShader::createShaderModule(VkShaderModule& module, const std::vector<char>& code, VkDevice device)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	return vkCreateShaderModule(device, &createInfo, nullptr, &module) == VK_SUCCESS;
}

void VkShader::release(VkDevice device)
{
	vkDestroyShaderModule(device, vertShader, nullptr);
	vkDestroyShaderModule(device, fragShader, nullptr);
}

void VkShader::ensureDefaultShader(VkDevice device)
{
	if (globalShaderList.size() == 0)
	{
		auto defaultShader = VkShader(device, ShaderSource::getDefaultShader());
		globalShaderList.push_back(MAKEUNQ<VkShader>(defaultShader));
	}
}

void VkShader::releaseGlobalShaderList(VkDevice device)
{
	if (globalShaderList.size() == 0)
		return;

	for (auto& shader : globalShaderList)
	{
		shader->release(device);
	}

	globalShaderList.clear();
}
