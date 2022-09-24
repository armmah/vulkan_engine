#pragma once
#include "pch.h"
#include "Common.h"
#include "VkTypes/VkMaterialVariant.h"

struct Mesh;
struct VkMesh;

class Material;
struct Transform;
struct TextureSource;
struct VkTexture2D;
struct VkMaterial;
struct VkMeshRenderer;
struct Path;

namespace Presentation
{
	class Device;
	class PresentationTarget;
}

typedef size_t MeshIndex;
typedef std::vector<TextureSource> SubmeshMaterials;

typedef size_t MaterialID;
typedef size_t IndexCount;

struct SerializedScene 
{
	std::vector<Mesh> m_meshes;
	std::vector<Material> m_materials;
	std::vector<Renderer> m_rendererIDs;
};

#include "Profiling/ProfileMarker.h"
class Scene
{
public:
	Scene(const Presentation::Device* device, Presentation::PresentationTarget* target);
	~Scene();

	const std::vector<Mesh>& getMeshes() const;
	const std::vector<Transform>& getTransforms() const;
	const std::vector<Material>& getMaterials() const;
	const std::vector<Renderer>& getRendererIDs() const;
	const std::vector<VkMesh>& getGraphicsMeshes() const;
	const std::vector<VkMeshRenderer>& getRenderers() const;

	bool load(VkDescriptorPool descPool);
	void release(VkDevice device, const VmaAllocator& allocator);

	bool tryLoadSupportedFormat(const Path& path);
	bool tryLoadFromFile(const Path& path, VkDescriptorPool descPool);

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ProfileMarker _("Scene::Serialize");

		ar& m_meshes
			& m_materials
			& m_rendererIDs
			& m_transforms;
	}

private:
	static bool loadOBJ_Implementation(std::vector<Mesh>& meshes, std::vector<Material>& materials, std::vector<Renderer>& rendererIDs, std::vector<Transform>& transforms, const std::string& path, const std::string& name);

	const Presentation::Device* m_presentationDevice;
	Presentation::PresentationTarget* m_presentationTarget;

	// Serialized scene data
	std::vector<Mesh> m_meshes;
	std::vector<Material> m_materials;
	std::vector<Renderer> m_rendererIDs;
	std::vector<Transform> m_transforms;

	// Graphics data
	std::vector<VkMeshRenderer> m_renderers;
	std::vector<UNQ<VkTexture2D>> m_textures;
	std::vector<UNQ<VkMaterial>> m_graphicsMaterials;
	std::vector<VkMesh> m_graphicsMeshes;
};
 