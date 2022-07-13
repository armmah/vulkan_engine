#pragma once
#include "pch.h"
#include "Common.h"

struct MeshDescriptor
{
public:
	static constexpr size_t descriptorCount = 4;

	MeshDescriptor() : lengths(), elementByteSizes()
	{
		lengths[0] = 1; lengths[1] = 1; lengths[2] = 1; lengths[3] = 1;

		elementByteSizes[0] = sizeof(glm::vec3); elementByteSizes[1] = sizeof(glm::vec2);
		elementByteSizes[2] = sizeof(glm::vec3); elementByteSizes[3] = sizeof(glm::vec3);
	}

	size_t lengths[descriptorCount];
	size_t elementByteSizes[descriptorCount];

	bool operator ==(const MeshDescriptor& other) const;
	bool operator !=(const MeshDescriptor& other) const;
};

struct VertexBinding
{
public:
	VertexBinding() = delete;

	VertexBinding(const MeshDescriptor& meshDescriptor);

	VertexBinding(const VkVertexInputBindingDescription& bindingDescription, std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);

	bool runValidations() const;
	VkPipelineVertexInputStateCreateInfo getVertexInputCreateInfo() const { return getTriangleMeshVertexBuffer(bindingDescription, attributeDescriptions); }
	
	static VkPipelineVertexInputStateCreateInfo getHardcodedTriangle();
	static VkPipelineVertexInputStateCreateInfo getTriangleMeshVertexBuffer(const VkVertexInputBindingDescription& bindingDescription, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);
	static bool validateAttributeAndBindingDescriptions(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);

private:
	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
};


