#pragma once
#include "pch.h"
#include "Common.h"
#include "VkTypes/VkMaterialVariant.h"

struct Mesh;
struct VkMesh;

class Material;
struct TextureSource;
struct VkTexture2D;
struct VkMaterial;
struct VkMeshRenderer;

namespace Presentation
{
	class Device;
	class PresentationTarget;
}

typedef size_t MeshIndex;
typedef std::vector<TextureSource> SubmeshMaterials;

typedef size_t MaterialID;
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

	std::vector<VkMeshRenderer> getRenderers() { return m_renderers; }

	bool tryLoadSupportedFormat(const std::string& path);
	bool tryLoadFromFile(const std::string& path, VkDescriptorPool descPool);

private:
	static bool loadOBJ_Implementation(std::vector<UNQ<Mesh>>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, const std::string& path, const std::string& name);
	static bool loadGLTF_Implementation(std::vector<UNQ<Mesh>>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, const std::string& path, const std::string& name, bool isBinary);

	const Presentation::Device* m_presentationDevice;
	Presentation::PresentationTarget* m_presentationTarget;

	// Serialized scene data
	std::vector<UNQ<Mesh>> m_meshes;
	std::vector<Material> m_materials;
	std::vector<Renderer> m_rendererIDs;

	// Graphics data
	std::vector<VkMeshRenderer> m_renderers;
	std::vector<UNQ<VkTexture2D>> m_textures;
	std::vector<UNQ<VkMaterial>> m_graphicsMaterials;
	std::vector<UNQ<VkMesh>> m_graphicsMeshes;
};
 