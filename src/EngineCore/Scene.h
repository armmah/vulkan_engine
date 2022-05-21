#pragma once
#include <vector>
#include <memory>
#include <algorithm>

#include "glm.hpp"
#include "vulkan/vulkan.h"

#include "vk_mem_alloc.h"

struct Color
{
private:
	glm::vec4 normalized;

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
};

struct DeprecatedVertex
{
	glm::vec2 pos;
	glm::vec3 col;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(DeprecatedVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(DeprecatedVertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(DeprecatedVertex, col);

		return attributeDescriptions;
	}
};

struct VertexBinding
{
	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	VkPipelineVertexInputStateCreateInfo getVertexInputCreateInfo() { return getTriangleMeshVertexBuffer(bindingDescription, attributeDescriptions); }

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

	static VkPipelineVertexInputStateCreateInfo getTriangleMeshVertexBuffer(VkVertexInputBindingDescription& bindingDescription, std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		return vertexInputInfo;
	}
};

struct VertexAttributes
{
	std::vector<VkBuffer> buffers;
	std::vector<VmaAllocation> memoryRanges;
	std::vector<VkDeviceSize> offsets;
	uint32_t firstBinding;
	uint32_t bindingCount;
	
	VertexAttributes() { }
	VertexAttributes(std::vector<VkBuffer>& vertexBuffers, std::vector<VmaAllocation> vertexMemoryRanges, std::vector<VkDeviceSize>& vertexOffsets, uint32_t bindingOffset = 0, uint32_t bindingCount = 0) :
		buffers(std::move(vertexBuffers)), memoryRanges(std::move(vertexMemoryRanges)), offsets(std::move(vertexOffsets)), firstBinding(bindingOffset), bindingCount(bindingCount)
	{
		assert(buffers.size() == offsets.size() && "vertex buffer size should match the vertex offsets size.");

		if (bindingCount == 0)
		{
			this->bindingCount = static_cast<uint32_t>(buffers.size());
		}
	}
};

struct IndexAttributes
{
	VkBuffer buffer;
	VmaAllocation memoryRange;
	VkDeviceSize offset;
	VkIndexType indexType;

	IndexAttributes() { }
	IndexAttributes(VkBuffer indexBuffer, VmaAllocation indexBufferMemory, VkIndexType indexType, VkDeviceSize offset = 0)
		: buffer(indexBuffer), memoryRange(indexBufferMemory), offset(offset), indexType(indexType) { }
};

struct VkMesh
{
	VertexAttributes vAttributes;
	uint32_t vCount;

	IndexAttributes iAttributes;
	uint32_t iCount;
};

constexpr std::size_t operator "" _z(unsigned long long n)
{
	return n;
}

struct Mesh
{
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;

	std::vector<uint16_t> indices;

	Mesh() noexcept :
		positions(),
		uvs(),
		normals(), 
		colors(), 
		indices() { }

	Mesh(Mesh&& other)  noexcept :
		positions(std::move(other.positions)),
		uvs(std::move(other.uvs)),
		normals(std::move(other.normals)),
		colors(std::move(other.colors)),
		indices(std::move(other.indices)) { }

	void clear()
	{
		positions.clear();
		uvs.clear();
		normals.clear();
		colors.clear();

		indices.clear();
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
		auto n = positions.size();

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

		if (!validateOptionalBufferSize(uvs.size(), n, "Uvs") ||
			!validateOptionalBufferSize(normals.size(), n, "Normals") ||
			!validateOptionalBufferSize(colors.size(), n, "Colors"))
			return false;

#ifndef NDEBUG
		for(size_t i = 0, in = indices.size(); i < in; ++i)
		{
			auto index = indices[i];
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
	uint32_t vectorsizeof(const typename std::vector<T>& vec)
	{
		return sizeof(T) * vec.size();
	}

	template<typename T>
	uint32_t normalizedSize(const typename std::vector<T>& vec)
	{
		return std::clamp(vec.size(), 0_z, 1_z);
	}

	template<typename T>
	uint32_t vectorElementsizeof(const typename std::vector<T>& vec)
	{
		return sizeof(T);// *normalizedSize(vec);
	}

	VkFormat pickDataFormat(size_t size)
	{
		static const VkFormat formats[] {
			VkFormat::VK_FORMAT_R32_SFLOAT,
			VkFormat::VK_FORMAT_R32G32_SFLOAT,
			VkFormat::VK_FORMAT_R32G32B32_SFLOAT,
			VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT,
		};

		return formats[size / sizeof(float) - 1];
	}

	template<typename T>
	static void copyInterleaved(std::vector<float>& interleavedVertexData, const std::vector<T>& srcVector, size_t iterStride, size_t offset)
	{
		if (srcVector.empty() || iterStride <= 0) // no op
			return;

		auto elementByteSize = sizeof(T);

		int i = 0;
		auto it = interleavedVertexData.begin();
		auto end = interleavedVertexData.end();
		std::advance(it, offset);

		for (;it != end;)
		{
			memcpy(&(*it), &srcVector[i], elementByteSize);
			if (std::distance(it, end) < iterStride)
				break;

			std::advance(it, iterStride);
			++i;
		}
	}

	void allocateVertexAttributes(VkMesh& graphicsMesh, VertexBinding& vbinding, VmaAllocator vmaAllocator)
	{
		uint32_t vertCount = positions.size();

		uint32_t p_size = vectorElementsizeof(positions),
			uv_size = vectorElementsizeof(uvs),
			n_size = vectorElementsizeof(normals),
			c_size = vectorElementsizeof(colors);

		uint32_t descriptorCount = 4;

		uint32_t p_stride = normalizedSize(positions) * p_size,
			uv_stride = normalizedSize(uvs) * uv_size,
			n_stride = normalizedSize(normals) * n_size,
			c_stride = normalizedSize(colors) * c_size;

		uint32_t vertexStride = p_stride + uv_stride + n_stride + c_stride;
		uint32_t floatCount = vertexStride / sizeof(float);
		uint32_t totalSizeBytes = vertexStride * vertCount;

#pragma region buffer_allocation
		VmaAllocationCreateInfo vmaACI{};
		vmaACI.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.size = totalSizeBytes;

		VkBuffer vBuffer;
		VmaAllocation vMemRange;
		vmaCreateBuffer(vmaAllocator, &bufferInfo, &vmaACI, &vBuffer, &vMemRange, nullptr);

		std::vector<VkBuffer> vBuffers{ vBuffer };
		std::vector<VkDeviceSize> vOffsets{ 0 };
		std::vector<VmaAllocation> vMemRanges{ vMemRange };
		graphicsMesh.vAttributes = VertexAttributes(vBuffers, vMemRanges, vOffsets);
		graphicsMesh.vCount = static_cast<uint32_t>(vertCount);
#pragma endregion

		// Align and optimize the mesh data
		auto interleavedCount = floatCount * vertCount;//totalSize / sizeof(float);
		std::vector<float> interleavedVertexData(interleavedCount);

		uint32_t offset = 0;
		copyInterleaved(interleavedVertexData, positions,	floatCount, offset);
		offset += p_stride / sizeof(float);

		copyInterleaved(interleavedVertexData, uvs,			floatCount, offset);
		offset += uv_stride / sizeof(float);

		copyInterleaved(interleavedVertexData, normals,		floatCount, offset);
		offset += n_stride / sizeof(float);

		copyInterleaved(interleavedVertexData, colors,		floatCount, offset);
		offset += c_stride / sizeof(float);

		/*
		int i = 0;
		for(auto it = interleavedVertexData.begin(); it != interleavedVertexData.end();)
		{
			memcpy(&(*it), &positions[i], p_size);
			std::advance(it, p_size / sizeof(float));

			auto sz = uv_size * std::clamp(uvs.size(), 0_z, 1_z);
			if (uvs.size() > 0)
			{
				memcpy(&(*it), &uvs[i], sz);
				std::advance(it, sz / sizeof(float));
			}

			sz = n_size * std::clamp(normals.size(), 0_z, 1_z);
			if (normals.size() > 0)
			{
				memcpy(&(*it), &normals[i], sz);
				std::advance(it, sz / sizeof(float));
			}

			sz = c_size * std::clamp(colors.size(), 0_z, 1_z);
			if (colors.size() > 0)
			{
				memcpy(&(*it), &colors[i], sz);
				std::advance(it, sz / sizeof(float));
			}

			++i;
		}*/

		// Hardcoded locations to match the shader locations for now
		std::vector<VkVertexInputAttributeDescription>& viaDesc = vbinding.attributeDescriptions;
		viaDesc.resize(descriptorCount);
		
		offset = 0;
		if (p_size > 0)
		{
			viaDesc[0].binding = 0;
			viaDesc[0].location = 0;
			viaDesc[0].format = pickDataFormat(p_size);
			viaDesc[0].offset = offset;

			offset += p_size;
		}

		if (uv_size > 0)
		{
			viaDesc[1].binding = 0;
			viaDesc[1].location = 1;
			viaDesc[1].format = pickDataFormat(uv_size);
			viaDesc[1].offset = offset;

			if (uvs.size() > 0)
				offset += uv_size;
		}

		if (n_size > 0)
		{
			viaDesc[2].binding = 0;
			viaDesc[2].location = 2;
			viaDesc[2].format = pickDataFormat(n_size);
			viaDesc[2].offset = offset;

			if (normals.size() > 0)
				offset += n_size;
		}

		if (c_size > 0)
		{
			viaDesc[3].binding = 0;
			viaDesc[3].location = 3;
			viaDesc[3].format = pickDataFormat(c_size);
			viaDesc[3].offset = offset;

			if (colors.size() > 0)
				offset += c_size;
		}

		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = offset;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		vbinding.bindingDescription = bindingDescription;
		
		/*
		1. Validate (vertexBindingDescriptionCount <= VkPhysicalDeviceLimits::maxVertexInputBindings)
		2. Validate (vertexAttributeDescriptionCount <= VkPhysicalDeviceLimits::maxVertexInputAttributes)
		3. Foreach attributeDescriptions[i].binding validate that bindingDescription with the same binding exists
		4. bindingDescription array should not have conflicting bindings (ensure unique)
		5. attributeDescriptions array should not have conflicting locations (ensure unique)
		*/

		// Fill the vertex buffer
		void* data;
		vmaMapMemory(vmaAllocator, vMemRange, &data);
		memcpy(data, interleavedVertexData.data(), totalSizeBytes);
		vmaUnmapMemory(vmaAllocator, vMemRange);

		printf("Copied vertex buffer of size: %i elements and %i bytes (%f bytes per vert).\n", vertCount, totalSizeBytes, totalSizeBytes / (double)vertCount);
	}

	void allocateIndexAttributes(VkMesh& graphicsMesh, VmaAllocator vmaAllocator)
	{
		size_t totalSize = vectorsizeof(indices);

		VmaAllocationCreateInfo vmaACI{};
		vmaACI.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		bufferInfo.size = static_cast<uint32_t>(totalSize);

		VkBuffer iBuffer;
		VmaAllocation iMemRange;
		vmaCreateBuffer(vmaAllocator, &bufferInfo, &vmaACI, &iBuffer, &iMemRange, nullptr);

		graphicsMesh.iAttributes = IndexAttributes(iBuffer, iMemRange, VkIndexType::VK_INDEX_TYPE_UINT16);
		graphicsMesh.iCount = static_cast<uint32_t>(indices.size());
		
		// Fill the index buffer
		void* data;
		vmaMapMemory(vmaAllocator, iMemRange, &data);
		memcpy(data, indices.data(), totalSize);
		vmaUnmapMemory(vmaAllocator, iMemRange);

		printf("Copied index buffer of size: %i elements and %i bytes (%f bytes per index).\n", indices.size(), totalSize, totalSize / (double)indices.size());
	}

	void allocateGraphicsMesh(VkMesh& graphicsMesh, VertexBinding& vertexBindings, VmaAllocator vmaAllocator)
	{
		allocateVertexAttributes(graphicsMesh, vertexBindings, vmaAllocator);

		allocateIndexAttributes(graphicsMesh, vmaAllocator);
	}

	static Mesh getPrimitiveTriangle()
	{
		Mesh mesh;

		float extent = 0.5f;
		mesh.positions = {
			{ 0.0f, -extent, 0.0f },
			{ extent, extent, 0.0f },
			{ -extent, extent, 0.0f },
		};

		mesh.uvs = {
			{ 0.5, 0 },
			{ 1.0, 1.0 },
			{ 0.0, 1.0 }
		};

		glm::vec3 forward(0.0f, 0.0f, 1.0f);
		mesh.normals = {
			forward,
			forward,
			forward
		};

		mesh.colors = {
			Color::red().v4(),
			Color::green().v4(),
			Color::blue().v4()
		};

		mesh.indices = {
			0, 1, 2
		};

		return mesh;
	}

	static Mesh getPrimitiveQuad()
	{
		Mesh mesh;

		float extent = 0.5f;
		mesh.positions = {
			{ -extent, -extent, 0.0f },
			{ extent, -extent, 0.0f },
			{ -extent, extent, 0.0f },
			{ extent, extent, 0.0f }
		};

		//mesh.uvs = {
		//	{ 0.0, 0.0 },
		//	{ 1.0, 0.0 },
		//	{ 0.0, 1.0 },
		//	{ 1.0, 1.0 }
		//};

		//glm::vec3 forward(0.0f, 0.0f, 1.0f);
		//mesh.normals = {
		//	forward,
		//	forward,
		//	forward,
		//	forward
		//};

		mesh.colors = {
			Color::red().v4(),
			Color::green().v4(),
			Color::green().v4(),
			Color::blue().v4()
		};

		mesh.indices = {
			0, 1, 2,
			2, 1, 3
		};

		return mesh;
	}
};

class Scene
{
	// hardcoded to contain a single mesh for now
	std::unique_ptr<Mesh> mesh;

public:
	VkMesh graphicsMesh;
	VertexBinding vertexBinding;

	Scene() { }


	bool load(VmaAllocator vmaAllocator)
	{
		mesh = std::make_unique<Mesh>(Mesh::getPrimitiveQuad());
		mesh->allocateGraphicsMesh(graphicsMesh, vertexBinding, vmaAllocator);

		return mesh->isValid();
	}

	void release()
	{
		mesh->clear();
		mesh.release();
	}
};
 