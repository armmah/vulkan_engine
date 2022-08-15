#pragma once
#include "pch.h"
#include "Common.h"
#include "VkTypes/VkMaterialVariant.h"

struct Mesh;
struct VkMesh;

class Material;
struct VkTexture2D;

namespace Presentation
{
	class Device;
	class PresentationTarget;
}

class Scene
{
public:
	Scene(const Presentation::Device* device, Presentation::PresentationTarget* target);
	~Scene();

	const std::vector<UNQ<Mesh>>& getMeshes() const;
	const std::vector<UNQ<VkMesh>>& getGraphicsMeshes() const;

	bool load(const VmaAllocator& vmaAllocator, VkDescriptorPool descPool);
	void release(VkDevice device, const VmaAllocator& allocator);

	std::vector<MeshRenderer> getRenderers() { return renderers; }

	bool tryLoadFromFile(std::vector<UNQ<Mesh>>& meshes, std::vector<UNQ<VkTexture2D>>& textures, std::unordered_map<uint32_t, std::vector<uint32_t>>& meshTextureMap, const std::string& path);

private:
	bool loadObjImplementation(std::vector<UNQ<Mesh>>& meshes, std::vector<UNQ<VkTexture2D>>& textures, std::unordered_map<uint32_t, std::vector<uint32_t>>& meshTextureMap, const std::string& path, const std::string& name);

	const Presentation::Device* presentationDevice;
	Presentation::PresentationTarget* presentationTarget;

	std::vector<UNQ<Mesh>> meshes;
	std::vector<UNQ<Material>> materials;
	std::vector<UNQ<VkTexture2D>> textures;
	std::vector<UNQ<VkMesh>> graphicsMeshes;
	std::vector<MeshRenderer> renderers;
};
 