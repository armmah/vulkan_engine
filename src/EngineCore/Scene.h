#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <set>
#include <stdexcept>

#include "glm.hpp"
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
	size_t vectorsizeof(const typename std::vector<T>& vec)
	{
		return sizeof(T) * vec.size();
	}

	template<typename T>
	size_t normalizedSize(const typename std::vector<T>& vec)
	{
		return std::clamp(vec.size(), 0_z, 1_z);
	}

	template<typename T>
	size_t vectorElementsizeof(const typename std::vector<T>& vec)
	{
		return sizeof(T);
	}

	template<typename T>
	size_t sumValues(const typename T* arr, size_t length)
	{
		size_t total = 0;
		for (size_t i = 0; i < length; i++)
			total += arr[i];
		return total;
	}

	VkFormat pickDataFormat(size_t size)
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

	bool allocateVertexAttributes(VkMesh& graphicsMesh, UNQ<VertexBinding>& vbinding, const VmaAllocator& vmaAllocator)
	{
		size_t vertCount = positions.size();
		const size_t descriptorCount = 4;

		size_t p_size = vectorElementsizeof(positions),
			uv_size = vectorElementsizeof(uvs),
			n_size = vectorElementsizeof(normals),
			c_size = vectorElementsizeof(colors);

		size_t byteSizes[descriptorCount] = { p_size, uv_size, n_size, c_size };
		size_t strides[descriptorCount] = { normalizedSize(positions) * p_size, normalizedSize(uvs) * uv_size, normalizedSize(normals) * n_size, normalizedSize(colors) * c_size };
		void* vectors[descriptorCount] = { positions.data(), uvs.data(), normals.data(), colors.data() };
		size_t vectorSizes[descriptorCount] = { positions.size(), uvs.size(), normals.size(), colors.size() };

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

		initializeBindings(vbinding, descriptorCount, byteSizes, vectorSizes);
		
		if(!vbinding->runValidations())
			return false;

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

	bool allocateIndexAttributes(VkMesh& graphicsMesh, const VmaAllocator& vmaAllocator)
	{
		size_t totalSize = vectorsizeof(indices);

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
		graphicsMesh.iCount = as_uint32(indices.size());
		
		// Fill the index buffer
		mapAndCopyBuffer(vmaAllocator, iMemRange, indices.data(), indices.size() / 3, totalSize, 
			"Copied index buffer of size: %zu triangles and %zu bytes (%f bytes per triangle).\n");

		return true;
	}

	bool allocateGraphicsMesh(UNQ<VkMesh>& graphicsMesh, UNQ<VertexBinding>& vertexBindings, const VmaAllocator& vmaAllocator)
	{
		graphicsMesh = MAKEUNQ<VkMesh>();
		if (!allocateVertexAttributes(*graphicsMesh, vertexBindings, vmaAllocator) ||
			!allocateIndexAttributes(*graphicsMesh, vmaAllocator))
			return false;

		return true;
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

private:
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;

	std::vector<uint16_t> indices;
};

class Scene
{
public:
	const Mesh& getMesh() const { return *mesh; }
	const VkMesh& getGraphicsMesh() const { return *graphicsMesh; }
	const VertexBinding& getVertexBinding() const { return *vertexBinding; }

	bool load(const VmaAllocator& vmaAllocator)
	{
		mesh = MAKEUNQ<Mesh>(Mesh::getPrimitiveQuad());

		return mesh->allocateGraphicsMesh(graphicsMesh, vertexBinding, vmaAllocator) && 
			mesh->isValid();
	}

	void release(const VmaAllocator& allocator)
	{
		mesh->clear();
		mesh.release();

		graphicsMesh->release(allocator);
		vertexBinding.release();
	}

private:
	// hardcoded to contain a single mesh for now
	UNQ<Mesh> mesh;
	UNQ<VkMesh> graphicsMesh;
	UNQ<VertexBinding> vertexBinding;
};
 