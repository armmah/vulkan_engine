#pragma once

#pragma warning ( disable : 26812 )
#include <vector>
#include <memory>
#include <algorithm>
#include <set>
#include <stdexcept>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "glm/gtc/quaternion.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "vulkan/vulkan.h"

#include "vk_mem_alloc.h"
#include "Common.h"

struct Color
{
public:
	Color(glm::vec4 normalizedColor) : normalized(normalizedColor) { }
	Color(float r, float g, float b) : normalized(glm::vec4(r, g, b, 1.0f)) { }
	Color(float r, float g, float b, float a) : normalized(glm::vec4(r, g, b, a)) { }

	glm::vec4 v4() const { return normalized; }

	static Color white()	{ return Color(1.0f, 1.0f, 1.0f); }
	static Color black()	{ return Color(0.0f, 0.0f, 0.0f); }
	static Color red()		{ return Color(1.0f, 0.0f, 0.0f); }
	static Color green()	{ return Color(0.0f, 1.0f, 0.0f); }
	static Color blue()		{ return Color(0.0f, 0.0f, 1.0f); }

	operator glm::vec4&() { return normalized; }
	operator glm::vec4() const { return normalized; }

private:
	glm::vec4 normalized;
};

struct VertexBinding
{
public:
	VertexBinding() = delete;
	VertexBinding(const VkVertexInputBindingDescription& bindingDescription, std::vector<VkVertexInputAttributeDescription>& attributeDescriptions) 
		: bindingDescription(bindingDescription), attributeDescriptions(std::move(attributeDescriptions)) { }

	VkPipelineVertexInputStateCreateInfo getVertexInputCreateInfo() const { return getTriangleMeshVertexBuffer(bindingDescription, attributeDescriptions); }

	static VkPipelineVertexInputStateCreateInfo getHardcodedTriangle()
	{
		// Vertex data - hacking in hardcoded vertex shader defined triangle
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		return vertexInputInfo;
	}

	static VkPipelineVertexInputStateCreateInfo getTriangleMeshVertexBuffer(const VkVertexInputBindingDescription& bindingDescription, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = as_uint32(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		return vertexInputInfo;
	}

	bool runValidations()
	{
		return validateAttributeAndBindingDescriptions(
			{ bindingDescription },
			attributeDescriptions
		);
	}

	static bool validateAttributeAndBindingDescriptions(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		//1. Validate (vertexBindingDescriptionCount <= VkPhysicalDeviceLimits::maxVertexInputBindings)
		//2. Validate (vertexAttributeDescriptionCount <= VkPhysicalDeviceLimits::maxVertexInputAttributes)

		// 3. bindingDescription array should not have conflicting bindings (ensure unique)
		std::set<uint32_t> uniqueBindings;
		for (auto& bindDesc : bindingDescriptions)
		{
			if (uniqueBindings.count(bindDesc.binding))
			{
				printf("A conflicting binding was detected in the binding descriptions.");
				return false;
			}
			uniqueBindings.insert(bindDesc.binding);
		}

		// 4. Foreach attributeDescriptions[i].binding validate that bindingDescription with the same binding exists
		for (auto& attrDesc : attributeDescriptions)
		{
			if (uniqueBindings.count(attrDesc.binding) == 0)
			{
				printf("The required binding could not be found.");
				return false;
			}
		}

		// 5. attributeDescriptions array should not have conflicting locations (ensure unique)
		std::set<uint32_t> uniqueLocations;
		for (auto& attrDesc : attributeDescriptions)
		{
			if (uniqueLocations.count(attrDesc.location))
			{
				printf("A conflicting location was detected in the vertex attribute descriptions.");
				return false;
			}

			uniqueLocations.insert(attrDesc.location);
		}

		return true;
	}

private:
	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
};

struct VertexAttributes
{
public:
	VertexAttributes() = delete;
	VertexAttributes(std::vector<VkBuffer>& vertexBuffers, std::vector<VmaAllocation> vertexMemoryRanges, std::vector<VkDeviceSize>& vertexOffsets, uint32_t bindingOffset = 0, uint32_t bindingCount = 0) :
		buffers(std::move(vertexBuffers)), memoryRanges(std::move(vertexMemoryRanges)), offsets(std::move(vertexOffsets)), firstBinding(bindingOffset), bindingCount(bindingCount)
	{
		assert(buffers.size() == offsets.size() && "vertex buffer size should match the vertex offsets size.");

		if (bindingCount == 0)
		{
			this->bindingCount = as_uint32(buffers.size());
		}
	}

	void bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, buffers.data(), offsets.data());
	}

	void destroy(const VmaAllocator& allocator)
	{
		for (int i = 0; i < memoryRanges.size(); i++)
		{
			vmaDestroyBuffer(allocator, buffers[i], memoryRanges[i]);
		}
	}

private:
	std::vector<VkBuffer> buffers;
	std::vector<VmaAllocation> memoryRanges;
	std::vector<VkDeviceSize> offsets;
	uint32_t firstBinding;
	uint32_t bindingCount;
};

struct IndexAttributes
{
public:
	IndexAttributes() = delete;
	IndexAttributes(const VkBuffer& indexBuffer, const VmaAllocation& indexBufferMemory, VkIndexType indexType, VkDeviceSize offset = 0)
		: buffer(indexBuffer), memoryRange(indexBufferMemory), offset(offset), indexType(indexType) { }

	void bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
	}

	void destroy(const VmaAllocator& allocator)
	{
		vmaDestroyBuffer(allocator, buffer, memoryRange);
	}

private:
	VkBuffer buffer;
	VmaAllocation memoryRange;
	VkDeviceSize offset;
	VkIndexType indexType;
};

struct VkMesh
{
public:
	VkMesh() : vAttributes(nullptr), vCount(0), iAttributes(nullptr), iCount(0) { }

	void release(const VmaAllocator& allocator)
	{
		vAttributes->destroy(allocator);
		iAttributes->destroy(allocator);
	}

	UNQ<VertexAttributes> vAttributes;
	uint32_t vCount;

	UNQ<IndexAttributes> iAttributes;
	uint32_t iCount;
};

constexpr std::size_t operator "" _z(unsigned long long n)
{
	return n;
}

struct Mesh
{
public:
	Mesh(std::vector<glm::vec3>& positions, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& colors, std::vector<uint16_t>& indices)
		: m_positions(std::move(positions)), m_uvs(std::move(uvs)), m_normals(std::move(normals)), m_colors(std::move(colors)), m_indices(std::move(indices))
	{
		updateMetaData();
	}

	void clear()
	{
		m_positions.clear();
		m_uvs.clear();
		m_normals.clear();
		m_colors.clear();

		m_indices.clear();
	}

	static bool validateOptionalBufferSize(size_t vectorSize, size_t vertexCount, char const* name)
	{
		// Optional vector, but if it exists the size should match the position array.
		if (vectorSize > 0 && vectorSize != vertexCount)
		{
			printf("%s array size '%zu' does not match the position array size '%zu'.", name, vectorSize, vertexCount);
			return false;
		}

		return true;
	}

	bool isValid()
	{
		auto n = m_positions.size();

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
		for(size_t i = 0, in = m_indices.size(); i < in; ++i)
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

	template<typename T>
	static size_t vectorsizeof(const typename std::vector<T>& vec)
	{
		return sizeof(T) * vec.size();
	}

	template<typename T>
	static size_t normalizedSize(const typename std::vector<T>& vec)
	{
		return std::clamp(vec.size(), 0_z, 1_z);
	}

	template<typename T>
	static size_t vectorElementsizeof(const typename std::vector<T>& vec)
	{
		return sizeof(T);
	}

	template<typename T>
	static size_t sumValues(const typename T* arr, size_t length)
	{
		size_t total = 0;
		for (size_t i = 0; i < length; i++)
			total += arr[i];
		return total;
	}

	static VkFormat pickDataFormat(size_t size)
	{
		static const VkFormat formats[4] {
			VkFormat::VK_FORMAT_R32_SFLOAT,
			VkFormat::VK_FORMAT_R32G32_SFLOAT,
			VkFormat::VK_FORMAT_R32G32B32_SFLOAT,
			VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT,
		};

		auto index = size / sizeof(float) - 1;

		assert((size % sizeof(float) == 0) && index >= 0 && index < 4 && "The vertex attribute data type is not supported.");

		return formats[index];
	}

	static void copyInterleavedNoCheck(std::vector<float>& interleavedVertexData, const void* src, size_t elementByteSize, size_t iterStride, size_t offset)
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

	template<typename T>
	void mapAndCopyBuffer(VmaAllocator vmaAllocator, VmaAllocation memRange, const T* source, size_t elementCount, size_t totalByteSize, const char* message)
	{
		void* data;
		vmaMapMemory(vmaAllocator, memRange, &data);
		memcpy(data, source, totalByteSize);
		vmaUnmapMemory(vmaAllocator, memRange);

		printf(message, elementCount, totalByteSize, totalByteSize / (float)elementCount);
	}

	struct MeshDescriptor
	{
		static const size_t descriptorCount = 4;
		void* vectors[descriptorCount];
		size_t lengths[descriptorCount];
		size_t elementByteSizes[descriptorCount];

		bool operator ==(const MeshDescriptor& other) const
		{
			for (int i = 0; i < descriptorCount; i++)
			{
				if(elementByteSizes[i] != other.elementByteSizes[i] ||
					std::min(lengths[i], 1_z) != std::min(other.lengths[i], 1_z))
					return false;
			}
			return true;
		}

		bool operator !=(const MeshDescriptor& other) const { return !(*this == other); }
	};

	static void initializeBindings(UNQ<VertexBinding>& vbinding, const MeshDescriptor& meshDescriptor)
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

		vbinding = MAKEUNQ<VertexBinding>(bindingDescription, attributeDescriptions);
	}

	void initializeBindings(UNQ<VertexBinding>& vbinding) { initializeBindings(vbinding, metaData); }

	void initializeBindings(UNQ<VertexBinding>& vbinding, size_t descriptorCount, const size_t* byteSizes, const size_t* vectorSizes)
	{
		VkVertexInputBindingDescription bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		// Vertex Attribute Descriptions
		attributeDescriptions.resize(descriptorCount);

		size_t offset = 0;
		for (uint32_t i = 0; i < descriptorCount; i++)
		{
			auto size = byteSizes[i];

			attributeDescriptions[i].binding = 0;
			attributeDescriptions[i].location = i;
			attributeDescriptions[i].format = pickDataFormat(size);
			attributeDescriptions[i].offset = as_uint32(offset);

			if (vectorSizes[i] > 0)
				offset += size;
		}

		bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = as_uint32(offset);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		vbinding = MAKEUNQ<VertexBinding>(bindingDescription, attributeDescriptions);
	}

	bool allocateBufferAndMemory(VkBuffer& vBuffer, VmaAllocation& vMemRange, const VmaAllocator& vmaAllocator, uint32_t totalSizeBytes)
	{
		VmaAllocationCreateInfo vmaACI{};
		vmaACI.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.size = totalSizeBytes;

		return vmaCreateBuffer(vmaAllocator, &bufferInfo, &vmaACI, &vBuffer, &vMemRange, nullptr) == VK_SUCCESS;
	}

	bool allocateVertexAttributes(VkMesh& graphicsMesh, const VmaAllocator& vmaAllocator)
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
		allocateBufferAndMemory(vBuffer, vMemRange, vmaAllocator, as_uint32(totalSizeBytes));
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

			copyInterleavedNoCheck(interleavedVertexData, metaData.vectors[i], byteSizes[i], floatCount, offset);
			offset += strides[i] / sizeof(float);
		}

		// Fill the vertex buffer
		mapAndCopyBuffer(vmaAllocator, vMemRange, interleavedVertexData.data(), vertCount, totalSizeBytes, 
			"Copied vertex buffer of size: %zu elements and %zu bytes (%f bytes per vertex).\n");

		return true;
	}

	bool allocateIndexAttributes(VkMesh& graphicsMesh, const VmaAllocator& vmaAllocator)
	{
		size_t totalSize = vectorsizeof(m_indices);

		VmaAllocationCreateInfo vmaACI{};
		vmaACI.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		bufferInfo.size = as_uint32(totalSize);

		VkBuffer iBuffer;
		VmaAllocation iMemRange;
		if(vmaCreateBuffer(vmaAllocator, &bufferInfo, &vmaACI, &iBuffer, &iMemRange, nullptr) != VK_SUCCESS)
			return false;

		graphicsMesh.iAttributes = MAKEUNQ<IndexAttributes>(iBuffer, iMemRange, VkIndexType::VK_INDEX_TYPE_UINT16);
		graphicsMesh.iCount = as_uint32(m_indices.size());
		
		// Fill the index buffer
		mapAndCopyBuffer(vmaAllocator, iMemRange, m_indices.data(), m_indices.size() / 3, totalSize, 
			"Copied index buffer of size: %zu triangles and %zu bytes (%f bytes per triangle).\n");

		return true;
	}

	bool allocateGraphicsMesh(UNQ<VkMesh>& graphicsMesh, const VmaAllocator& vmaAllocator)
	{
		graphicsMesh = MAKEUNQ<VkMesh>();
		if (!allocateVertexAttributes(*graphicsMesh, vmaAllocator) ||
			!allocateIndexAttributes(*graphicsMesh, vmaAllocator))
			return false;

		return true;
	}

	static Mesh getPrimitiveTriangle()
	{
		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> colors;
		std::vector<uint16_t> indices;

		float extentX = 0.5f;
		float extentY = extentX / 3.0f;
		positions = {
			{ 0.0f, -extentY * 2.f, 0.0f },
			{ extentX, extentY, 0.0f },
			{ -extentX, extentY, 0.0f },
		};

		uvs = {
			{ 0.5, 0 },
			{ 1.0, 1.0 },
			{ 0.0, 1.0 }
		};

		glm::vec3 forward(0.0f, 0.0f, 1.0f);
		normals = {
			forward,
			forward,
			forward
		};

		colors = {
			Color::red().v4() * 0.5f + glm::vec4(0.5f),
			Color::green().v4() * 0.5f + glm::vec4(0.5f),
			Color::blue().v4() * 0.5f + glm::vec4(0.5f)
		};

		indices = {
			0, 1, 2
		};

		return Mesh(positions, uvs, normals, colors, indices);
	}

	static Mesh getPrimitiveQuad()
	{
		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> colors;
		std::vector<uint16_t> indices;

		float extent = 0.5f;
		positions = {
			{ -extent, -extent, 0.0f },
			{ extent, -extent, 0.0f },
			{ -extent, extent, 0.0f },
			{ extent, extent, 0.0f }
		};

		uvs = {
			{ 0.0, 0.0 },
			{ 1.0, 0.0 },
			{ 0.0, 1.0 },
			{ 1.0, 1.0 }
		};

		glm::vec3 forward(0.0f, 0.0f, 1.0f);
		normals = {
			forward,
			forward,
			forward,
			forward
		};

		colors = {
			Color::red().v4() * 0.5f,
			Color::green().v4() * 0.5f,
			Color::green().v4() * 0.5f,
			Color::blue().v4() * 0.5f
		};

		indices = {
			0, 1, 2,
			2, 1, 3
		};

		return Mesh(positions, uvs, normals, colors, indices);
	}

	void makeFace(glm::vec3 pivot, glm::vec3 up, glm::vec3 right, uint16_t firstIndex)
	{
		m_positions.push_back(pivot + up - right);
		m_positions.push_back(pivot + up + right);
		m_positions.push_back(pivot - up - right);
		m_positions.push_back(pivot - up + right);

		m_uvs.push_back({ 0.0, 0.0 });
		m_uvs.push_back({ 1.0, 0.0 });
		m_uvs.push_back({ 0.0, 1.0 });
		m_uvs.push_back({ 1.0, 1.0 });

		auto normal = glm::normalize(pivot);
		m_normals.push_back(normal);
		m_normals.push_back(normal);
		m_normals.push_back(normal);
		m_normals.push_back(normal);

		m_colors.push_back(Color::red().v4());
		m_colors.push_back(Color::green().v4());
		m_colors.push_back(Color::green().v4());
		m_colors.push_back(Color::blue().v4());

		uint16_t indices[6] {
			0, 1, 2,
			2, 1, 3
		};
		if (normal.x < 0 || normal.y < 0 || normal.z < 0)
		{
			for (int i = 0; i < 6; i += 1)
				m_indices.push_back(firstIndex + indices[i]);
		}
		else
		{
			for (int i = 5; i >= 0; i -= 1)
				m_indices.push_back(firstIndex + indices[i]);
		}
	}

	static Mesh getPrimitiveCube()
	{
		uint16_t vertPerFace = 4u;
		uint16_t indexPerFace = 6u;
		uint16_t numOfFaces = 6u;
		Mesh mesh(vertPerFace * numOfFaces, indexPerFace * numOfFaces);
		
		glm::vec3 right(0.5f, 0.f, 0.f);
		glm::vec3 up(0.f, 0.5f, 0.f);
		glm::vec3 forward(0.f, 0.f, 0.5f);

		mesh.makeFace(right, up, -forward,	vertPerFace * 0);
		mesh.makeFace(-right, up, -forward, vertPerFace * 1);
		mesh.makeFace(up, right, forward,	vertPerFace * 2);
		mesh.makeFace(-up, right, forward,	vertPerFace * 3);
		mesh.makeFace(forward, up, right,	vertPerFace * 4);
		mesh.makeFace(-forward, up, right,	vertPerFace * 5);

		mesh.updateMetaData();
		return mesh;
	}

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
	void updateMetaData()
	{
		metaData.vectors[0] = m_positions.data();
		metaData.vectors[1] = m_uvs.data();
		metaData.vectors[2] = m_normals.data();
		metaData.vectors[3] = m_colors.data();

		metaData.lengths[0] = m_positions.size();
		metaData.lengths[1] = m_uvs.size();
		metaData.lengths[2] = m_normals.size();
		metaData.lengths[3] = m_colors.size();

		metaData.elementByteSizes[0] = vectorElementsizeof(m_positions);
		metaData.elementByteSizes[1] = vectorElementsizeof(m_uvs);
		metaData.elementByteSizes[2] = vectorElementsizeof(m_normals);
		metaData.elementByteSizes[3] = vectorElementsizeof(m_colors);
	}
};

class Scene
{
public:
	const std::vector<UNQ<Mesh>>& getMeshes() const { return meshes; }
	const std::vector<UNQ<VkMesh>>& getGraphicsMeshes() const { return graphicsMeshes; }
	const VertexBinding& getVertexBinding() const { return *vertexBinding; }

	bool load(const VmaAllocator& vmaAllocator)
	{
		meshes.resize(2);
		meshes[0] = MAKEUNQ<Mesh>(Mesh::getPrimitiveTriangle());
		meshes[1] = MAKEUNQ<Mesh>(Mesh::getPrimitiveCube());

		auto count = meshes.size();
		graphicsMeshes.resize(count);
		Mesh::MeshDescriptor md;
		for(int i = 0; i < count; i++)
		{
			if (i == 0)
				md = meshes[i]->getMeshDescriptor();

			if (!meshes[i]->allocateGraphicsMesh(graphicsMeshes[i], vmaAllocator) || !meshes[i]->isValid())
				return false;

			if (md != meshes[i]->getMeshDescriptor())
			{
				printf("Mesh metadata does not match - can not bind to the same pipeline.");
				return false;
			}
		}
		
		Mesh::initializeBindings(vertexBinding, md);

		return vertexBinding->runValidations();
	}

	void release(const VmaAllocator& allocator)
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

		vertexBinding.release();
	}

private:
	std::vector<UNQ<Mesh>> meshes;
	std::vector<UNQ<VkMesh>> graphicsMeshes;
	UNQ<VertexBinding> vertexBinding;
};
 