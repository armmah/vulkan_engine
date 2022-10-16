#include "pch.h"
#include "CollectionUtility.h"
#include "VertexBinding.h"
#include "Mesh.h"

#include "VkTypes/InitializersUtility.h"

VertexBinding::VertexBinding(const MeshDescriptor& meshDescriptor)
{
	auto descriptorCount = meshDescriptor.descriptorCount;
	// Vertex Attribute Descriptions
	attributeDescriptions.resize(descriptorCount);

	size_t offset = 0;
	for (uint32_t i = 0; i < descriptorCount; i++)
	{
		auto size = meshDescriptor.elementByteSizes[i];

		attributeDescriptions[i].binding = 0;
		attributeDescriptions[i].location = i;
		attributeDescriptions[i].format = Mesh::pickDataFormat(size);
		attributeDescriptions[i].offset = as_uint32(offset);

		offset += size * std::clamp(meshDescriptor.lengths[i], 0_z, 1_z);
	}

	bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = as_uint32(offset);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

VertexBinding::VertexBinding(const VkVertexInputBindingDescription& bindingDescription, std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	: bindingDescription(bindingDescription), attributeDescriptions(std::move(attributeDescriptions)) { }

VkPipelineVertexInputStateCreateInfo VertexBinding::getHardcodedTriangle()
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

VkPipelineVertexInputStateCreateInfo VertexBinding::getTriangleMeshVertexBuffer(const VkVertexInputBindingDescription& bindingDescription, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = as_uint32(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	return vertexInputInfo;
}

bool VertexBinding::runValidations() const
{
	return validateAttributeAndBindingDescriptions(
		{ bindingDescription },
		attributeDescriptions
	);
}

bool VertexBinding::validateAttributeAndBindingDescriptions(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
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
