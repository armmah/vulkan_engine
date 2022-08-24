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

typedef int32_t MeshIndex;
typedef std::vector<std::string> SubmeshMaterials;

typedef int32_t MaterialID;
typedef size_t IndexCount;

class Scene
{
public:
	Scene(const Presentation::Device* device, Presentation::PresentationTarget* target);
	~Scene();

	const std::vector<UNQ<Mesh>>& getMeshes() const;
	const std::vector<UNQ<VkMesh>>& getGraphicsMeshes() const;

	bool load(VkDescriptorPool descPool);
	void release(VkDevice device, const VmaAllocator& allocator);

	std::vector<MeshRenderer> getRenderers() { return m_renderers; }

	bool tryLoadTestScene_1(VkDescriptorPool descPool);
	bool tryLoadSupportedFormat(const std::string& path, std::vector<UNQ<Mesh>>& meshes, std::unordered_map<MeshIndex, SubmeshMaterials>& meshTextureMap);
	bool tryLoadFromFile(const std::string& path, VkDescriptorPool descPool);

private:
	bool loadOBJ_Implementation(std::vector<UNQ<Mesh>>& meshes, std::unordered_map<MeshIndex, SubmeshMaterials>& meshTextureMap, const std::string& path, const std::string& name);
	bool loadGLTF_Implementation(std::vector<UNQ<Mesh>>& meshes, std::unordered_map<MeshIndex, SubmeshMaterials>& meshTextureMap, const std::string& path, const std::string& name, bool isBinary);

	const Presentation::Device* m_presentationDevice;
	Presentation::PresentationTarget* m_presentationTarget;

	std::vector<UNQ<Mesh>> m_meshes;
	std::vector<UNQ<Material>> m_materials;
	std::vector<UNQ<VkTexture2D>> m_textures;
	std::vector<UNQ<VkMesh>> m_graphicsMeshes;
	std::vector<MeshRenderer> m_renderers;
};
 