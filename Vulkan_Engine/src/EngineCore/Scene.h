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

	std::vector<MeshRenderer> getRenderers() { return m_renderers; }

	bool tryLoadTestScene_1(VkDescriptorPool descPool);
	bool tryLoadFromFile(const std::string& path, std::unordered_map<uint32_t, std::vector<uint32_t>>& meshTextureMap, VkDescriptorPool descPool);

private:
	bool loadObjImplementation(std::vector<UNQ<Mesh>>& meshes, std::vector<UNQ<VkTexture2D>>& textures, std::unordered_map<uint32_t, std::vector<uint32_t>>& meshTextureMap, const std::string& path, const std::string& name);

	const Presentation::Device* m_presentationDevice;
	Presentation::PresentationTarget* m_presentationTarget;

	std::vector<UNQ<Mesh>> m_meshes;
	std::vector<UNQ<Material>> m_materials;
	std::vector<UNQ<VkTexture2D>> m_textures;
	std::vector<UNQ<VkMesh>> m_graphicsMeshes;
	std::vector<MeshRenderer> m_renderers;
};
 