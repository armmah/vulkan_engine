#pragma once
#include "pch.h"

struct VkMesh;
class Material;
struct BoundsAABB;
struct Transform;
struct VkMaterialVariant;

struct VkMeshRenderer
{
	const VkMesh* mesh;
	const VkMaterialVariant* variant;
	const BoundsAABB* bounds;
	const Transform* transform;
	uint32_t submeshIndex;

	const Material* material;

	VkMeshRenderer() = default;
	VkMeshRenderer(const VkMeshRenderer& other) = default;
	VkMeshRenderer& operator=(VkMeshRenderer const& other) = default;
	VkMeshRenderer(VkMeshRenderer&& other) = default;

	VkMeshRenderer(const VkMesh* mesh, const Material* material, const VkMaterialVariant* variant, const BoundsAABB* bounds, Transform* transform);
	VkMeshRenderer(const VkMesh* mesh, uint32_t submeshIndex, const Material* material, const VkMaterialVariant* variant, const BoundsAABB* bounds, Transform* transform);
};