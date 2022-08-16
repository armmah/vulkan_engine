#pragma once
#include "pch.h"
#include "Common.h"

struct MeshDescriptor
{
	using TVertexIndices = uint16_t;

	using TVertexPosition = glm::vec3;
	using TVertexUV = glm::vec2;
	using TVertexNormal = glm::vec3;
	using TVertexColor = glm::vec3;

	enum EAttributePresent
	{
		Positions = 1,
		UVs = 1,
		Normals = 1,
		Colors = 0
	};

	static constexpr size_t descriptorCount = 4;

	MeshDescriptor() : lengths(), elementByteSizes()
	{
		lengths[0] = EAttributePresent::Positions; lengths[1] = EAttributePresent::UVs; lengths[2] = EAttributePresent::Normals; lengths[3] = EAttributePresent::Colors;

		elementByteSizes[0] = sizeof(TVertexPosition); elementByteSizes[1] = sizeof(TVertexUV);
		elementByteSizes[2] = sizeof(TVertexNormal); elementByteSizes[3] = sizeof(TVertexColor);
	}

	size_t lengths[descriptorCount];
	size_t elementByteSizes[descriptorCount];

	bool operator ==(const MeshDescriptor& other) const;
	bool operator !=(const MeshDescriptor& other) const;
};

struct VertexBinding
{
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
