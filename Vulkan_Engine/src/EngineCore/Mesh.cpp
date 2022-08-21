#include "pch.h"
#include "Common.h"
#include "CollectionUtility.h"
#include "VkMesh.h"
#include "Mesh.h"
#include "VkTypes/InitializersUtility.h"
#include "VertexAttributes.h"
#include "IndexAttributes.h"

Mesh::Mesh(std::vector<glm::vec3>& positions, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors, std::vector<SubMesh>& submeshes)
	: m_positions(std::move(positions)), m_uvs(std::move(uvs)), m_normals(std::move(normals)), m_colors(std::move(colors)), m_submeshes(std::move(submeshes))
{
	updateMetaData();
}

Mesh::Mesh(std::vector<glm::vec3>& positions, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors, SubMesh& submesh)
	: m_positions(std::move(positions)), m_uvs(std::move(uvs)), m_normals(std::move(normals)), m_colors(std::move(colors)), m_submeshes(1)
{
	m_submeshes[0] = std::move(submesh);
	updateMetaData();
}

Mesh::Mesh(std::vector<glm::vec3>& positions, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors, SubMesh&& submesh)
	: m_positions(std::move(positions)), m_uvs(std::move(uvs)), m_normals(std::move(normals)), m_colors(std::move(colors)), m_submeshes(1)
{
	m_submeshes[0] = std::move(submesh);
	updateMetaData();
}

Mesh::Mesh(size_t vertN, size_t indexN)
{
	m_positions.reserve(vertN);
	m_uvs.reserve(vertN);
	m_normals.reserve(vertN);
	m_colors.reserve(vertN);

	m_submeshes.resize(1);
	m_submeshes[0].m_indices.reserve(indexN);

	updateMetaData();
}

void Mesh::clear()
{
	m_positions.clear();
	m_uvs.clear();
	m_normals.clear();
	m_colors.clear();

	m_submeshes.clear();
}

bool Mesh::validateOptionalBufferSize(size_t vectorSize, size_t vertexCount, char const* name)
{
	// Optional vector, but if it exists the size should match the position array.
	if (vectorSize > 0 && vectorSize != vertexCount)
	{
		printf("%s array size '%zu' does not match the position array size '%zu'.\n", name, vectorSize, vertexCount);
		return false;
	}

	return true;
}

bool Mesh::isValid()
{
	const auto n = m_positions.size();

	if (n == 0)
	{
		printf("The vertex array can not have 0 length.\n");
		return false;
	}

	// We won't support more than 65535 verts for now due to 16bit indexing
	if (n >= std::numeric_limits<uint16_t>::max())
	{
		printf("The vertex array size '%zu' exceeds the allowed capacity '%i'.\n", n, std::numeric_limits<uint16_t>::max());
		return false;
	}

	if (!validateOptionalBufferSize(m_uvs.size(), n, "Uvs") ||
		!validateOptionalBufferSize(m_normals.size(), n, "Normals") ||
		!validateOptionalBufferSize(m_colors.size(), n, "Colors"))
		return false;

#ifndef NDEBUG
	for (auto& submesh : m_submeshes)
	{
		for (size_t i = 0, size = submesh.m_indices.size(); i < size; ++i)
		{
			auto index = submesh.m_indices[i];
			if (index < 0 || index >= n)
			{
				printf("An incorrect index '%i' detected at position indices[%zu], should be in {0, %zu} range.\n", index, i, n);
				return false;
			}
		}
	}
#endif

	return true;
}

bool MeshDescriptor::operator ==(const MeshDescriptor& other) const
{
	for (int i = 0; i < descriptorCount; i++)
	{
		if (elementByteSizes[i] != other.elementByteSizes[i] ||
			std::min(lengths[i], 1_z) != std::min(other.lengths[i], 1_z))
			return false;
	}
	return true;
}

bool MeshDescriptor::operator !=(const MeshDescriptor& other) const { return !(*this == other); }

template<typename T>
inline void Mesh::mapAndCopyBuffer(const VmaAllocator& vmaAllocator, VmaAllocation& memRange, const T* source, size_t elementCount, size_t totalByteSize, const char* message)
{
	void* data;
	vmaMapMemory(vmaAllocator, memRange, &data);
	memcpy(data, source, totalByteSize);
	vmaUnmapMemory(vmaAllocator, memRange);

#ifdef VERBOSE_INFO_MESSAGE
	printf(message, elementCount, totalByteSize, totalByteSize / static_cast<float>(elementCount));
#endif
}

bool Mesh::allocateVertexAttributes(VkMesh& graphicsMesh, const VmaAllocator& vmaAllocator)
{
	size_t vertCount = m_positions.size();

	const size_t descriptorCount = metaData.descriptorCount;
	auto& byteSizes = metaData.elementByteSizes;
	auto& vectorSizes = metaData.lengths;

	size_t strides[descriptorCount];
	for (int i = 0; i < descriptorCount; i++)
		strides[i] = std::clamp(metaData.lengths[i], 0_z, 1_z) * byteSizes[i];

	size_t vertexStride = sumValues(strides, descriptorCount);
	size_t floatCount = vertexStride / sizeof(float);
	size_t totalSizeBytes = vertexStride * vertCount;

	VkBuffer vBuffer;
	VmaAllocation vMemRange;
	if (!vkinit::MemoryBuffer::allocateBufferAndMemory(vBuffer, vMemRange, vmaAllocator, as_uint32(totalSizeBytes), VMA_MEMORY_USAGE_CPU_TO_GPU, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT))
	{
		printf("Could not allocate vertex memory buffer.\n");
		return false;
	}

	std::vector<VkBuffer> vBuffers{ vBuffer };
	std::vector<VkDeviceSize> vOffsets{ 0 };
	std::vector<VmaAllocation> vMemRanges{ vMemRange };
	graphicsMesh.vAttributes = MAKEUNQ<VertexAttributes>(vBuffers, vMemRanges, vOffsets);
	graphicsMesh.vCount = as_uint32(vertCount);

	// Align and optimize the mesh data
	auto interleavedCount = floatCount * vertCount;
	std::vector<float> interleavedVertexData(interleavedCount);

	size_t offset = 0;
	for (int i = 0; i < descriptorCount; i++)
	{
		if (vectorSizes[i] == 0)
			continue;

		copyInterleavedNoCheck(interleavedVertexData, vectors[i], byteSizes[i], floatCount, offset);
		offset += strides[i] / sizeof(float);
	}

	// Fill the vertex buffer
	mapAndCopyBuffer(vmaAllocator, vMemRange, interleavedVertexData.data(), vertCount, totalSizeBytes,
		"Copied vertex buffer of size: %zu elements and %zu bytes (%f bytes per vertex).\n");

	return true;
}

bool Mesh::allocateIndexAttributes(VkMesh& graphicsMesh, const SubMesh& submesh, const VmaAllocator& vmaAllocator)
{
	size_t totalSize = vectorsizeof(submesh.m_indices);

	VkBuffer iBuffer;
	VmaAllocation iMemRange;
	if (!vkinit::MemoryBuffer::allocateBufferAndMemory(iBuffer, iMemRange, vmaAllocator, as_uint32(totalSize), VMA_MEMORY_USAGE_CPU_TO_GPU, VK_BUFFER_USAGE_INDEX_BUFFER_BIT))
	{
		printf("Could not allocate index memory buffer.\n");
		return false;
	}

	auto indexCount = submesh.getIndexCount();
	graphicsMesh.iAttributes.push_back(
		IndexAttributes(iBuffer, iMemRange, as_uint32(indexCount), VkIndexType::VK_INDEX_TYPE_UINT16)
	);

	// Fill the index buffer
	mapAndCopyBuffer(vmaAllocator, iMemRange, submesh.m_indices.data(), indexCount / 3, totalSize,
		"Copied index buffer of size: %zu triangles and %zu bytes (%f bytes per triangle).\n");

	return true;
}

VkFormat Mesh::pickDataFormat(size_t size)
{
	static const VkFormat formats[4]{
		VkFormat::VK_FORMAT_R32_SFLOAT,
		VkFormat::VK_FORMAT_R32G32_SFLOAT,
		VkFormat::VK_FORMAT_R32G32B32_SFLOAT,
		VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT,
	};

	auto index = size / sizeof(float) - 1;

	assert((size % sizeof(float) == 0) && index >= 0 && index < 4 && "The vertex attribute data type is not supported.");

	return formats[index];
}

void Mesh::copyInterleavedNoCheck(std::vector<float>& interleavedVertexData, const void* src, size_t elementByteSize, size_t iterStride, size_t offset)
{
	int i = 0;
	auto it = interleavedVertexData.begin();
	auto end = interleavedVertexData.end();
	std::advance(it, offset);

	while (it != end)
	{
		memcpy(&(*it), (char*)src + i * elementByteSize, elementByteSize);
		if (std::distance(it, end) < static_cast<ptrdiff_t>(iterStride))
			break;

		std::advance(it, iterStride);
		++i;
	}
}

VertexBinding Mesh::initializeBindings(const MeshDescriptor& meshDescriptor)
{
	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	auto descriptorCount = meshDescriptor.descriptorCount;
	// Vertex Attribute Descriptions
	attributeDescriptions.resize(descriptorCount);

	size_t offset = 0;
	for (uint32_t i = 0; i < descriptorCount; i++)
	{
		auto size = meshDescriptor.elementByteSizes[i];

		attributeDescriptions[i].binding = 0;
		attributeDescriptions[i].location = i;
		attributeDescriptions[i].format = pickDataFormat(size);
		attributeDescriptions[i].offset = as_uint32(offset);

		offset += size * std::clamp(meshDescriptor.lengths[i], 0_z, 1_z);
	}

	bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = as_uint32(offset);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return VertexBinding(bindingDescription, attributeDescriptions);
}

bool Mesh::allocateGraphicsMesh(UNQ<VkMesh>& graphicsMesh, const VmaAllocator& vmaAllocator)
{
	graphicsMesh = MAKEUNQ<VkMesh>();
	if (!allocateVertexAttributes(*graphicsMesh, vmaAllocator))
		return false;

	for (auto& submesh : m_submeshes)
	{
		if (!allocateIndexAttributes(*graphicsMesh, submesh, vmaAllocator))
			return false;
	}

	return true;
}

void Mesh::updateMetaData()
{
	vectors[0] = m_positions.data();
	vectors[1] = m_uvs.data();
	vectors[2] = m_normals.data();
	vectors[3] = m_colors.data();

	metaData.lengths[0] = m_positions.size();
	metaData.lengths[1] = m_uvs.size();
	metaData.lengths[2] = m_normals.size();
	metaData.lengths[3] = m_colors.size();

	metaData.elementByteSizes[0] = vectorElementsizeof(m_positions);
	metaData.elementByteSizes[1] = vectorElementsizeof(m_uvs);
	metaData.elementByteSizes[2] = vectorElementsizeof(m_normals);
	metaData.elementByteSizes[3] = vectorElementsizeof(m_colors);
}

