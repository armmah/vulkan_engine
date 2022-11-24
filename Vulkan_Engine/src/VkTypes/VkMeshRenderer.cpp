#include "pch.h"
#include "VkMeshRenderer.h"
#include "Mesh.h"
#include "PresentationTarget.h"
#include "Material.h"

VkMeshRenderer::VkMeshRenderer(const VkMesh* mesh, const Material* material, const VkMaterialVariant* variant, const BoundsAABB* bounds, Transform* transform)
	: mesh(mesh), submeshIndex(0), material(material), variant(variant), bounds(bounds), transform(transform) {}

VkMeshRenderer::VkMeshRenderer(const VkMesh* mesh, uint32_t submeshIndex, const Material* material, const VkMaterialVariant* variant, const BoundsAABB* bounds, Transform* transform)
	: mesh(mesh), submeshIndex(submeshIndex), material(material), variant(variant), bounds(bounds), transform(transform) {}
