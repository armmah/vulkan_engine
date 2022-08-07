#pragma once
#include "pch.h"
#include "Common.h"

class Material;
struct VkMesh;
struct VkGraphicsPipeline;

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
	const VkPipeline pipeline;
	const VkPipelineLayout pipelineLayout;
	const VkDescriptorSetLayout descriptorSetLayout;

	std::array<VkDescriptorSet, SWAPCHAIN_IMAGE_COUNT> descriptorSets;
};

struct MeshRenderer
{
	const VkMesh* mesh;
	const VkMaterialVariant* variant;
	uint32_t submeshIndex;

	MeshRenderer(const VkMesh* mesh, const VkMaterialVariant* variant) : mesh(mesh), submeshIndex(0), variant(variant) {}
	MeshRenderer(const VkMesh* mesh, uint32_t submeshIndex, const VkMaterialVariant* variant) : mesh(mesh), submeshIndex(submeshIndex), variant(variant) {}
};