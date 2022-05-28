#pragma once

#include <set>
#include "Common.h"

#include "vulkan/vulkan.h"

struct VertexBinding
{
public:
	VertexBinding() = delete;
	VertexBinding(const VkVertexInputBindingDescription& bindingDescription, std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
		: bindingDescription(bindingDescription), attributeDescriptions(std::move(attributeDescriptions)) { }

	VkPipelineVertexInputStateCreateInfo getVertexInputCreateInfo() const { return getTriangleMeshVertexBuffer(bindingDescription, attributeDescriptions); }

	static VkPipelineVertexInputStateCreateInfo getHardcodedTriangle()
	{
		// Vertex data - hacking in hardcoded vertex shader defined triangle
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		return vertexInputInfo;
	}

	static VkPipelineVertexInputStateCreateInfo getTriangleMeshVertexBuffer(const VkVertexInputBindingDescription& bindingDescription, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = as_uint32(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		return vertexInputInfo;
	}

	bool runValidations()
	{
		return validateAttributeAndBindingDescriptions(
			{ bindingDescription },
			attributeDescriptions
		);
	}

	static bool validateAttributeAndBindingDescriptions(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		//1. Validate (vertexBindingDescriptionCount <= VkPhysicalDeviceLimits::maxVertexInputBindings)
		//2. Validate (vertexAttributeDescriptionCount <= VkPhysicalDeviceLimits::maxVertexInputAttributes)

		// 3. bindingDescription array should not have conflicting bindings (ensure unique)
		std::set<uint32_t> uniqueBindings;
		for (auto& bindDesc : bindingDescriptions)
		{
			if (uniqueBindings.count(bindDesc.binding))
			{
				printf("A conflicting binding was detected in the binding descriptions.");
				return false;
			}
			uniqueBindings.insert(bindDesc.binding);
		}

		// 4. Foreach attributeDescriptions[i].binding validate that bindingDescription with the same binding exists
		for (auto& attrDesc : attributeDescriptions)
		{
			if (uniqueBindings.count(attrDesc.binding) == 0)
			{
				printf("The required binding could not be found.");
				return false;
			}
		}

		// 5. attributeDescriptions array should not have conflicting locations (ensure unique)
		std::set<uint32_t> uniqueLocations;
		for (auto& attrDesc : attributeDescriptions)
		{
			if (uniqueLocations.count(attrDesc.location))
			{
				printf("A conflicting location was detected in the vertex attribute descriptions.");
				return false;
			}

			uniqueLocations.insert(attrDesc.location);
		}

		return true;
	}

private:
	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
};


