#include "pch.h"
#include "Scene.h"
#include "Color.h"

#include "vk_mem_alloc.h"

#include "VertexBinding.h"
#include "VkMesh.h"
#include "Mesh.h"

Mesh::Mesh(std::vector<glm::vec3>& positions, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors, std::vector<uint16_t>& indices)
	: m_positions(std::move(positions)), m_uvs(std::move(uvs)), m_normals(std::move(normals)), m_colors(std::move(colors)), m_indices(std::move(indices))
{
	updateMetaData();
}

void Mesh::clear()
{
	m_positions.clear();
	m_uvs.clear();
	m_normals.clear();
	m_colors.clear();

	m_indices.clear();
}

bool Mesh::validateOptionalBufferSize(size_t vectorSize, size_t vertexCount, char const* name)
{
	// Optional vector, but if it exists the size should match the position array.
	if (vectorSize > 0 && vectorSize != vertexCount)
	{
		printf("%s array size '%zu' does not match the position array size '%zu'.", name, vectorSize, vertexCount);
		return false;
	}

	return true;
}

bool Mesh::isValid()
{
	const auto n = m_positions.size();

	if (n == 0)
	{
		printf("The vertex array can not have 0 length.");
		return false;
	}

	// We won't support more than 65535 verts for now due to 16bit indexing
	if (n >= std::numeric_limits<uint16_t>::max())
	{
		printf("The vertex array size '%zu' exceeds the allowed capacity '%i'.", n, std::numeric_limits<uint16_t>::max());
		return false;
	}

	if (!validateOptionalBufferSize(m_uvs.size(), n, "Uvs") ||
		!validateOptionalBufferSize(m_normals.size(), n, "Normals") ||
		!validateOptionalBufferSize(m_colors.size(), n, "Colors"))
		return false;

#ifndef NDEBUG
	for (size_t i = 0, size = m_indices.size(); i < size; ++i)
	{
		auto index = m_indices[i];
		if (index < 0 || index >= n)
		{
			printf("An incorrect index '%i' detected at position indices[%zu], should be in {0, %zu} range.", index, i, n);
			return false;
		}
	}
#endif

	return true;
}

bool Scene::load(const VmaAllocator& vmaAllocator)
{
	meshes.resize(1);
	Mesh::tryLoadFromFile(meshes[0], "C:/Git/Vulkan_Engine/Resources/dragon.obj");

	//meshes[0] = MAKEUNQ<Mesh>(Mesh::getPrimitiveCube());

	auto defaultMeshDescriptor = Mesh::defaultMeshDescriptor;

	const auto count = meshes.size();
	graphicsMeshes.resize(count);
	for (int i = 0; i < count; i++)
	{
		if (!meshes[i]->allocateGraphicsMesh(graphicsMeshes[i], vmaAllocator) || !meshes[i]->isValid())
			return false;

		if (defaultMeshDescriptor != meshes[i]->getMeshDescriptor())
		{
			printf("Mesh metadata does not match - can not bind to the same pipeline.");
			return false;
		}
	}

	return true;
}

void Scene::release(const VmaAllocator& allocator)
{
	for (auto& mesh : meshes)
	{
		mesh->clear();
		mesh.release();
	}

	for (auto& gmesh : graphicsMeshes)
	{
		gmesh->release(allocator);
	}
}

const std::vector<UNQ<Mesh>>& Scene::getMeshes() const { return meshes; }
const std::vector<UNQ<VkMesh>>& Scene::getGraphicsMeshes() const { return graphicsMeshes; }
