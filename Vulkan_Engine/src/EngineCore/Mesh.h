#pragma once

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

#include "Common.h"
#include "CollectionUtility.h"

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

#include "VkMesh.h"
#include "VertexBinding.h"

struct Mesh
{
public:
	Mesh(std::vector<glm::vec3>& positions, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors, std::vector<uint16_t>& indices)
		: m_positions(std::move(positions)), m_uvs(std::move(uvs)), m_normals(std::move(normals)), m_colors(std::move(colors)), m_indices(std::move(indices))
	{
		updateMetaData();
	}

	void clear();
	bool isValid();

	struct MeshDescriptor
	{
	public:
		static const size_t descriptorCount = 4;
		size_t lengths[descriptorCount];
		size_t elementByteSizes[descriptorCount];

		bool operator ==(const MeshDescriptor& other) const;
		bool operator !=(const MeshDescriptor& other) const;
	};

	template<typename T>
	void mapAndCopyBuffer(VmaAllocator vmaAllocator, VmaAllocation& memRange, const T* source, size_t elementCount, size_t totalByteSize, const char* message);

	static VkFormat pickDataFormat(size_t size);
	static bool validateOptionalBufferSize(size_t vectorSize, size_t vertexCount, char const* name);
	static void copyInterleavedNoCheck(std::vector<float>& interleavedVertexData, const void* src, size_t elementByteSize, size_t iterStride, size_t offset);
	static void initializeBindings(UNQ<VertexBinding>& vbinding, const MeshDescriptor& meshDescriptor);

	bool allocateGraphicsMesh(UNQ<VkMesh>& graphicsMesh, const VmaAllocator& vmaAllocator);
	bool allocateIndexAttributes(VkMesh& graphicsMesh, const VmaAllocator& vmaAllocator);
	bool allocateVertexAttributes(VkMesh& graphicsMesh, const VmaAllocator& vmaAllocator);

	void makeFace(glm::vec3 pivot, glm::vec3 up, glm::vec3 right, uint16_t firstIndex);
	static Mesh getPrimitiveCube();
	static Mesh getPrimitiveQuad();
	static Mesh getPrimitiveTriangle();

	const MeshDescriptor& getMeshDescriptor() { return metaData; }

private:
	Mesh(size_t vertN, size_t indexN)
	{
		m_positions.reserve(vertN);
		m_uvs.reserve(vertN);
		m_normals.reserve(vertN);
		m_colors.reserve(vertN);

		m_indices.reserve(indexN);

		updateMetaData();
	}

	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec2> m_uvs;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec3> m_colors;

	std::vector<uint16_t> m_indices;

	MeshDescriptor metaData;
	void* vectors[MeshDescriptor::descriptorCount];

	void updateMetaData();
};
