#pragma once
#include "pch.h"
#include "Common.h"

class Material;
struct VkMesh;
struct VkGraphicsPipeline;
struct BoundsAABB;

enum VariantStateChange : uint32_t
{
	None = 0,

	Pipeline = 1,

	DescriptorSet = 6
};

struct VkMaterialVariant
{
	VkMaterialVariant(const VkPipeline pipeline, const VkPipelineLayout pipelineLayout, const VkDescriptorSetLayout descriptorSetLayout, std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT>& descriptorSets);

	const VkPipeline getPipeline() const;
	const VkPipelineLayout getPipelineLayout() const;
	const VkDescriptorSet* getDescriptorSet(uint32_t frameNumber) const;
	const VkDescriptorSetLayout getDescriptorSetLayout() const;

	VariantStateChange compare(const VkMaterialVariant* other) const;

	void release(VkDevice device);

private:
	const VkPipeline m_pipeline;
	const VkPipelineLayout m_pipelineLayout;
	const VkDescriptorSetLayout m_descriptorSetLayout;

	std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT> m_descriptorSets;
};

namespace boost {
	namespace serialization {
		class access;
	}
}
struct Renderer
{
	size_t meshID;
	size_t transformID;
	std::vector<size_t> materialIDs;

	Renderer(size_t meshID, size_t transformID, std::vector<size_t>&& materialIDs)
		: meshID(meshID), transformID(transformID), materialIDs(materialIDs) { }
	Renderer(size_t meshID, size_t transformID, std::vector<size_t>& materialIDs)
		: meshID(meshID), transformID(transformID), materialIDs(std::move(materialIDs)) { }

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& meshID;
		ar& materialIDs;
	}

	bool operator ==(const Renderer& other) const
	{
		if (meshID != other.meshID || materialIDs.size() != other.materialIDs.size())
			return false;

		for (size_t i = 0; i < materialIDs.size(); i++)
		{
			if (materialIDs[i] != other.materialIDs[i])
				return false;
		}
		return true;
	}

private:
	friend class boost::serialization::access;
	Renderer() : meshID(), transformID() { }
};

struct Transform
{
	glm::mat4 localToWorld;

	Transform() : localToWorld(1.0f) { }
	Transform(const glm::mat4& mat) : localToWorld(mat) { }
	Transform(glm::mat4&& mat) : localToWorld(std::move(mat)) { }

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& localToWorld;
	}

	bool operator==(const Transform& other) const { return localToWorld == other.localToWorld; }
	bool operator!=(const Transform& other) const { return !(*this != other); }
};

struct VkMeshRenderer
{
	const VkMesh* mesh;
	const VkMaterialVariant* variant;
	const BoundsAABB* bounds;
	const Transform* transform;
	uint32_t submeshIndex;

	VkMeshRenderer(const VkMesh* mesh, const VkMaterialVariant* variant, const BoundsAABB* bounds, Transform* transform) : mesh(mesh), submeshIndex(0), variant(variant), bounds(bounds), transform(transform) {}
	VkMeshRenderer(const VkMesh* mesh, uint32_t submeshIndex, const VkMaterialVariant* variant, const BoundsAABB* bounds, Transform* transform) : mesh(mesh), submeshIndex(submeshIndex), variant(variant), bounds(bounds), transform(transform) {}
};