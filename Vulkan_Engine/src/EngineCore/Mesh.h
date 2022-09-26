#pragma once
#include "pch.h"
#include "Common.h"
#include "vk_mem_alloc.h"
#include "VertexBinding.h"
#include "Math/BoundsAABB.h"

struct VkMesh;
class StagingBufferPool;

namespace Presentation {
	class Device;
}
namespace boost {
	namespace serialization {
		class access;
	}
}

struct SubMesh
{
	SubMesh() : m_indices(), m_bounds() { }
	SubMesh(size_t size) : m_indices(), m_bounds() { m_indices.reserve(size); }
	SubMesh(std::vector<MeshDescriptor::TVertexIndices>& indices) : m_indices(std::move(indices)), m_bounds() { }
	size_t getIndexCount() const { return m_indices.size(); }

	std::vector<MeshDescriptor::TVertexIndices> m_indices;
	BoundsAABB m_bounds;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& m_indices;
		ar& m_bounds.center;
		ar& m_bounds.extents;
	}

	bool operator ==(const SubMesh& other) const
	{
		if (m_indices.size() != other.m_indices.size())
			return false;

		for (size_t i = 0; i < m_indices.size(); i++)
		{
			if (m_indices[i] != other.m_indices[i])
				return false;
		}

		return true;
	}

	bool operator !=(const SubMesh& other) const { return !(*this == other); }
};

struct Mesh
{
public:
	Mesh(const Mesh& mesh) : m_positions(mesh.m_positions), m_uvs(mesh.m_uvs), m_normals(mesh.m_normals), m_colors(mesh.m_colors), m_submeshes(mesh.m_submeshes) { updateMetaData(); }
	Mesh(std::vector<glm::vec3>& positions, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors, std::vector<SubMesh>& submeshes);
	Mesh(std::vector<glm::vec3>& positions, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors, SubMesh& submesh);
	Mesh(std::vector<glm::vec3>& positions, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors, SubMesh&& submesh);

	void clear();
	bool isValid();

	template<typename T>
	void mapAndCopyBuffer(const VmaAllocator& vmaAllocator, VmaAllocation& memRange, const T* source, size_t elementCount, size_t totalByteSize, const char* message);

	static VkFormat pickDataFormat(size_t size);
	static bool validateOptionalBufferSize(size_t vectorSize, size_t vertexCount, char const* name);
	static void copyInterleavedNoCheck(std::vector<float>& interleavedVertexData, const void* src, size_t elementByteSize, size_t iterStride, size_t offset);
	static VertexBinding initializeBindings(const MeshDescriptor& meshDescriptor);

	bool allocateGraphicsMesh(VkMesh& graphicsMesh, const VmaAllocator& vmaAllocator, const Presentation::Device* presentationDevice, StagingBufferPool& stagingPool);
	bool allocateIndexAttributes(VkMesh& graphicsMesh, const SubMesh& submesh, const VmaAllocator& vmaAllocator, const Presentation::Device* presentationDevice, StagingBufferPool& stagingPool);
	bool allocateVertexAttributes(VkMesh& graphicsMesh, const VmaAllocator& vmaAllocator, const Presentation::Device* presentationDevice, StagingBufferPool& stagingPool);

	void makeFace(glm::vec3 pivot, glm::vec3 up, glm::vec3 right, MeshDescriptor::TVertexIndices firstIndex);
	static Mesh getPrimitiveCube();
	static Mesh getPrimitiveQuad();
	static Mesh getPrimitiveTriangle();

	const MeshDescriptor& getMeshDescriptor() const { return metaData; }
	const BoundsAABB* getBounds(uint32_t submeshIndex) const { return submeshIndex >= 0 && submeshIndex < m_submeshes.size() ? &m_submeshes[submeshIndex].m_bounds : nullptr; }

	inline static MeshDescriptor defaultMeshDescriptor = MeshDescriptor();
	inline static VertexBinding defaultVertexBinding = VertexBinding(defaultMeshDescriptor);

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& m_positions;
		ar& m_uvs;
		ar& m_normals;
		ar& m_colors;
		ar& m_submeshes;

		updateMetaData();
	}
	bool operator==(const Mesh& other) const;

private:
	friend class boost::serialization::access;
	Mesh() : vectors(), metaData() {};
	Mesh(size_t vertN, size_t indexN);

	std::vector<MeshDescriptor::TVertexPosition> m_positions;
	std::vector<MeshDescriptor::TVertexUV> m_uvs;
	std::vector<MeshDescriptor::TVertexNormal> m_normals;
	std::vector<MeshDescriptor::TVertexColor> m_colors;

	std::vector<SubMesh> m_submeshes;

	MeshDescriptor metaData;
	void* vectors[MeshDescriptor::descriptorCount];

	void updateMetaData();
};