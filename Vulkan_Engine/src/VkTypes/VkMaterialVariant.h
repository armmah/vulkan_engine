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

struct Renderer
{
	size_t meshID;
	std::vector<size_t> materialIDs;

	Renderer(size_t meshID, std::vector<size_t>&&  materialIDs)
		: meshID(meshID), materialIDs(materialIDs) { }
	Renderer(size_t meshID, std::vector<size_t>& materialIDs)
		: meshID(meshID), materialIDs(std::move(materialIDs)) { }
};

struct VkMeshRenderer
{
	const VkMesh* mesh;
	const VkMaterialVariant* variant;
	const BoundsAABB* bounds;
	uint32_t submeshIndex;

	VkMeshRenderer(const VkMesh* mesh, const BoundsAABB* bounds, const VkMaterialVariant* variant) : mesh(mesh), bounds(bounds), submeshIndex(0), variant(variant) {}
	VkMeshRenderer(const VkMesh* mesh, uint32_t submeshIndex, const BoundsAABB* bounds, const VkMaterialVariant* variant) : mesh(mesh), submeshIndex(submeshIndex), bounds(bounds), variant(variant) {}
};