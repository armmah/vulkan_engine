#include "pch.h"
#include "Scene.h"
#include "VkMesh.h"
#include "Mesh.h"
#include "Material.h"
#include "VkTypes/VkTexture.h"
#include "VkTypes/VkShader.h"

#include "Presentation/Device.h"
#include "PresentationTarget.h"
#include "EngineCore/StagingBufferPool.h"

#include "FileManager/FileIO.h"
#include "FileManager/Directories.h"
#include "Profiling/ProfileMarker.h"

#include "Loaders/Model/Common.h"
#include "Loaders/Model/ModelLoaderOptions.h"
#include "Loaders/Model/Loader_OBJ.h"
#include "Loaders/Model/Loader_ASSIMP.h"

Scene::Scene(const Presentation::Device* device, Presentation::PresentationTarget* target)
	: m_presentationDevice(device), m_presentationTarget(target) { }

Scene::~Scene() { }

constexpr bool force_serialize_from_origin = false;
bool Scene::load(VkDescriptorPool descPool)
{
	//const auto modelOptions = Directories::getModels_DebrovicSponza();
	//const auto modelOptions = Directories::getModels_IntelSponza();
	const auto modelOptions = Directories::getModels_CrytekSponza();

	Path fullPath;
	/*****************************				IMPORT					****************************************/
	if (!Directories::tryGetBinaryIfExists(fullPath, modelOptions.front().filePath) || force_serialize_from_origin)
	{
		ProfileMarker _("Scene::import & serialize");
		Scene scene(nullptr, nullptr);

		for (auto& model : modelOptions)
		{
			auto meshCount = scene.getMeshes().size();
			scene.tryInitializeFromFile(model);
			
			if (scene.getMeshes().size() <= meshCount)
			{
				printf("No meshes loaded from %s.", model.filePath.c_str());
			}
		}

		{
			auto stream = std::fstream(fullPath, std::ios::out | std::ios::binary);
			boost::archive::binary_oarchive archive(stream);

			archive << scene;
			stream.close();
		}
	}

	/*****************************				LOAD BINARY					****************************************/
	ProfileMarker _("Scene::load");

	const auto binaryLoaderOptions = Loader::ModelLoaderOptions(std::move(fullPath), 1.0);
	if (!tryInitializeFromFile(binaryLoaderOptions))
	{
		printf("Could not load scene file '%s', it did not match any of the supported formats.\n", binaryLoaderOptions.filePath.c_str());
		return false;
	}
	printf("Initialized the scene with (renderers = %zi), (transforms = %zi), (meshes = %zi), (textures = %zi), (materials = %zi).\n", m_renderers.size(), m_transforms.size(), m_meshes.size(), m_textures.size(), m_materials.size());

	/*****************************				GRAPHICS					****************************************/
	createGraphicsRepresentation(descPool);

	return true;
}

const std::vector<Mesh>& Scene::getMeshes() const { return m_meshes; }
const std::vector<Transform>& Scene::getTransforms() const { return m_transforms; }
const std::vector<Material>& Scene::getMaterials() const { return m_materials; }
const std::vector<Renderer>& Scene::getRendererIDs() const { return m_rendererIDs; }
const std::vector<VkMesh>& Scene::getGraphicsMeshes() const { return m_graphicsMeshes; }
const std::vector<VkMeshRenderer>& Scene::getRenderers() const { return m_renderers; }

void Scene::release(VkDevice device, const VmaAllocator& allocator)
{
	for (auto& mesh : m_meshes)
	{
		mesh.clear();
	}
	m_meshes.clear();

	for (auto& gmesh : m_graphicsMeshes)
	{
		gmesh.release(allocator);
	}
	m_graphicsMeshes.clear();

	for (auto& tex : m_textures)
	{
		tex->release(device);
		tex.release();
	}
	m_textures.clear();

	for (auto& mat : m_graphicsMaterials)
	{
		mat->release(device);
		mat.release();
	}
	m_graphicsMaterials.clear();

	m_renderers.clear();
	m_materials.clear();
	m_rendererIDs.clear();
}

template<class Archive>
void Scene::serialize(Archive& ar, const unsigned int version)
{
	ProfileMarker _("Scene::Serialize");

	ar& m_meshes
		& m_materials
		& m_rendererIDs
		& m_transforms;
}

bool Scene::tryInitializeFromFile(const Loader::ModelLoaderOptions& modelOptions)
{
	const auto& path = modelOptions.filePath;

	/* ================ READ SERIALIZED BINARY =============== */
	if (Directories::isBinary(path) && path.fileExists())
	{
		ProfileMarker _("Loader::Binary");

		// Deserialize
		auto stream = std::fstream(path, std::ios::in | std::ios::binary);
		boost::archive::binary_iarchive archive(stream);

		archive >> *this;

		return true;
	}

	/* ================ READ FROM GLTF =============== */
	if (fileExists(path, ".gltf"))
	{
		ProfileMarker _("Loader::ASSIMP");
		return Loader::load_AssimpImplementation(m_meshes, m_materials, m_rendererIDs, m_transforms, modelOptions);
	}

	/* ================ READ FROM OBJ =============== */
	if (fileExists(path, ".obj"))
	{
		ProfileMarker _("Loader::Custom_OBJ");
		return Loader::loadOBJ_Implementation(m_meshes, m_materials, m_rendererIDs, m_transforms, modelOptions);
	}

	// Fallback
	if (fileExists(path))
	{
		ProfileMarker _("Loader::ASSIMP");
		return Loader::load_AssimpImplementation(m_meshes, m_materials, m_rendererIDs, m_transforms, modelOptions);
	}

	return false;
}

void Scene::createGraphicsRepresentation(VkDescriptorPool descPool)
{
	std::unordered_map<TextureSource, uint32_t> loadedTextures;
	StagingBufferPool stagingBufPool{};
	{
		ProfileMarker _("Scene::Create_Graphics_Materials");
		/* ================= CREATE TEXTURES ================*/
		/* ================= CREATE GRAPHICS MATERIALS ================*/
		m_graphicsMaterials.reserve(m_textures.size());
		auto device = m_presentationDevice->getDevice();
		for (auto& rendererIDs : m_rendererIDs)
		{
			for (auto& matIndex : rendererIDs.materialIDs)
			{
				const auto& mat = m_materials[matIndex];
				const auto& texSrc = mat.getTextureSource();
				if (loadedTextures.count(texSrc) == 0)
				{
					auto size = m_textures.size();
					m_textures.resize(size + 1);
					m_graphicsMaterials.resize(size + 1);
					if (VkTexture2D::tryCreateTexture(m_textures.back(), texSrc, m_presentationDevice, stagingBufPool))
					{
						loadedTextures[texSrc] = as_uint32(size);

						auto* shader = VkShader::findShader(mat.getShaderIdentifier());
						m_presentationTarget->createGraphicsMaterial(m_graphicsMaterials.back(), device, descPool, shader, m_textures.back().get());
					}
					else
					{
						rendererIDs.materialIDs = { 0 };

						m_textures.resize(size);
						m_graphicsMaterials.resize(size);
						printf("The texture at '%s' could not be loaded.\n", texSrc.path.c_str());
					}
				}
			}
		}
	}

	{
		ProfileMarker _("Scene::Create_Graphics_Meshes");
		/* ================= CREATE GRAPHICS MESHES ================*/
		auto defaultMeshDescriptor = Mesh::defaultMeshDescriptor;
		auto vmaAllocator = VkMemoryAllocator::getInstance()->m_allocator;
		const auto count = m_meshes.size();

		m_graphicsMeshes.reserve(count);
		m_renderers.reserve(count);
		for (auto& ids : m_rendererIDs)
		{
			auto& mesh = m_meshes[ids.meshID];

			auto newGraphicsMesh = VkMesh();
			if (!mesh.allocateGraphicsMesh(newGraphicsMesh, vmaAllocator, m_presentationDevice, stagingBufPool) || !mesh.isValid())
				continue;

			if (defaultMeshDescriptor != mesh.getMeshDescriptor())
			{
				printf("Mesh metadata does not match - can not bind to the same pipeline.\n");
				continue;
			}
			m_graphicsMeshes.push_back(std::move(newGraphicsMesh));

			uint32_t submeshIndex = 0;
			for (auto materialIDs : ids.materialIDs)
			{
				const auto& texPath = m_materials[materialIDs].getTextureSource();
				if (!loadedTextures.count(texPath) && loadedTextures[texPath] > 0 && loadedTextures[texPath] < m_graphicsMaterials.size())
				{
					printf("SceneLoader - The material not found for '%zu' mesh.\n", ids.meshID);
					continue;
				}

				m_renderers.push_back(
					VkMeshRenderer(
						&m_graphicsMeshes.back(), submeshIndex, &m_materials[materialIDs],
						&m_graphicsMaterials[loadedTextures[texPath]]->getMaterialVariant(),
						mesh.getBounds(submeshIndex), &m_transforms[ids.transformID]
					)
				);
				++submeshIndex;
			}
		}
	}
	stagingBufPool.releaseAllResources();
}
