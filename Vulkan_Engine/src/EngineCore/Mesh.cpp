#include "pch.h"
#include "Common.h"
#include "CollectionUtility.h"
#include "VkMesh.h"
#include "Mesh.h"
#include "VkTypes/InitializersUtility.h"
#include "VertexAttributes.h"
#include "IndexAttributes.h"
#include "tiny_obj_loader.h"
#include <unordered_map>

Mesh::Mesh(size_t vertN, size_t indexN)
{
	m_positions.reserve(vertN);
	m_uvs.reserve(vertN);
	m_normals.reserve(vertN);
	m_colors.reserve(vertN);

	m_indices.reserve(indexN);

	updateMetaData();
}

template <typename T>
void fillArrayWithDefaultValue(std::vector<T>& dst, size_t offset, size_t count)
{
	for (int i = offset; i < std::min(dst.size(), count); i++)
	{
		dst[i] = T();
	}
}

template <typename F, typename T>
void reinterpretCopy(std::vector<F>& src, std::vector<T>& dst)
{
	F* r_src = (F*)src.data();
	T* r_dst = (T*)dst.data();

	auto byteSize_src = src.size() * sizeof(F);
	auto byteSize_dst = dst.size() * sizeof(T);
	auto minSize = byteSize_dst < byteSize_src ? byteSize_dst: byteSize_src;

	// Copy the contents of src to dst until dst is filled.
	memcpy(r_dst, r_src, minSize);

	// Fill the rest, if necessary.
	//fillArrayWithDefaultValue(dst, minSize / sizeof(T), dst.size());
	for (int i = minSize / sizeof(T); i < dst.size(); i++)
	{
		dst[i] = T();
	}
}

bool loadObjAsMesh(UNQ<Mesh>& mesh, const std::string& path)
{
	//attrib will contain the vertex arrays of the file
	tinyobj::attrib_t attrib;
	//shapes contains the info for each separate object in the file
	std::vector<tinyobj::shape_t> shapes;
	//materials contains the information about the material of each shape, but we won't use it.
	std::vector<tinyobj::material_t> materials;

	//error and warning output from the load function
	std::string warn;
	std::string err;

	//load the OBJ file
	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), nullptr);

	//make sure to output the warnings to the console, in case there are issues with the file
	if (!warn.empty())
	{
		printf("warning: %s\n", warn.c_str());
	}

	//if we have any error, print it to the console, and break the mesh loading.
	//This happens if the file can't be found or is malformed
	if (!err.empty())
	{
		printf("Error: %s\n", err.c_str());
		return false;
	}

	auto vn = attrib.vertices.size() / (sizeof(MeshDescriptor::TVertexPosition) / sizeof(float));
	std::vector<MeshDescriptor::TVertexPosition> vertices(vn);
	reinterpretCopy(attrib.vertices, vertices);

	std::vector<MeshDescriptor::TVertexNormal> normals(vn);
	reinterpretCopy(attrib.normals, normals);

	std::vector<MeshDescriptor::TVertexUV> uvs(vn);
	reinterpretCopy(attrib.texcoords, uvs);

	std::vector<MeshDescriptor::TVertexColor> colors(vn);
	reinterpretCopy(attrib.colors, colors);

	auto& firstShapeIndices = shapes[0].mesh.indices;
	std::vector<uint16_t> indices(firstShapeIndices.size());

	for (size_t i = 0; i < firstShapeIndices.size(); i++)
	{
		indices[i] = firstShapeIndices[i].vertex_index;
	}
	mesh = MAKEUNQ<Mesh>(vertices, uvs, normals, colors, indices);
	printf("\tLoaded obj mesh '%s'.\n", shapes[0].name.c_str());

	for (size_t i = 1; i < shapes.size(); i++)
	{
		printf("\tSkipped submesh '%s' from '%s'.\n", shapes[i].name.c_str(), path.c_str());
	}

	return true;
}


bool loadObjAsMesh(std::vector<UNQ<Mesh>>& meshes, const std::string& path, const std::string& name)
{
	//attrib will contain the vertex arrays of the file
	tinyobj::attrib_t attrib;
	//shapes contains the info for each separate object in the file
	std::vector<tinyobj::shape_t> shapes;
	//materials contains the information about the material of each shape, but we won't use it.
	std::vector<tinyobj::material_t> materials;

	//error and warning output from the load function
	std::string warn;
	std::string err;

	//load the OBJ file
	auto objPath = (path + name + ".obj");

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objPath.c_str(), path.c_str());

	//make sure to output the warnings to the console, in case there are issues with the file
	if (!warn.empty())
	{
		printf("warning: %s\n", warn.c_str());
	}

	//if we have any error, print it to the console, and break the mesh loading.
	//This happens if the file can't be found or is malformed
	if (!err.empty())
	{
		printf("Error: %s\n", err.c_str());
		return false;
	}

	std::unordered_map<size_t, uint16_t> indexMapping;
	for (size_t i = 0; i < shapes.size(); i++)
	//for (size_t i = 0; i < std::min( static_cast<size_t>(5), shapes.size() ); i++)
	{
		auto& shapeIndices = shapes[i].mesh.indices;
		std::vector<uint16_t> indices(shapeIndices.size());

		auto index = 0;
		for (size_t i = 0; i < shapeIndices.size(); i++)
		{
			auto& vertIndex = shapeIndices[i].vertex_index;
			auto curIndex = index;

			if (indexMapping.count(vertIndex) == 0)
			{
				indexMapping[vertIndex] = index;
				++index;
			}
			else
			{
				curIndex = indexMapping[vertIndex];
			}

			indices[i] = curIndex;
		}

		std::set<size_t> uniqueVertIndices;
		std::vector<MeshDescriptor::TVertexPosition> vertices;
		std::vector<MeshDescriptor::TVertexNormal> normals;
		vertices.reserve(index);
		normals.reserve(index);
		for (size_t i = 0; i < shapeIndices.size(); i++)
		{
			auto vi = shapeIndices[i].vertex_index;
			auto ni = shapeIndices[i].normal_index;

			if (uniqueVertIndices.count(vi) > 0)
				continue;
			else
			{
				vertices.push_back(
					{
						attrib.vertices[vi * 3],
						attrib.vertices[vi * 3 + 1],
						attrib.vertices[vi * 3 + 2]
					}
				);

				normals.push_back(
					{
						attrib.normals[ni * 3],
						attrib.normals[ni * 3 + 1],
						attrib.normals[ni * 3 + 2],
					}
				);

				uniqueVertIndices.insert(vi);
			}
		}

		auto vn = vertices.size();

		std::vector<MeshDescriptor::TVertexUV> uvs(vn);
		fillArrayWithDefaultValue(uvs, 0, vn);
		
		std::vector<MeshDescriptor::TVertexColor> colors(vn);
		fillArrayWithDefaultValue(colors, 0, vn);

		/*
		auto vn = attrib.vertices.size() / (sizeof(MeshDescriptor::TVertexPosition) / sizeof(float));
		std::vector<MeshDescriptor::TVertexPosition> vertices(vn);
		reinterpretCopy(attrib.vertices, vertices);

		reinterpretCopy(attrib.normals, normals);

		reinterpretCopy(attrib.texcoords, uvs);

		reinterpretCopy(attrib.colors, colors);
		*/

		meshes.push_back(MAKEUNQ<Mesh>(vertices, uvs, normals, colors, indices));
	}
	printf("\tLoaded obj mesh '%s'.\n", shapes[0].name.c_str());

	//for (size_t i = 1; i < shapes.size(); i++)
	//{
	//	printf("\tSkipped submesh '%s' from '%s'.\n", shapes[i].name.c_str(), path.c_str());
	//}

	return true;
}

bool Mesh::tryLoadFromFile(std::vector<UNQ<Mesh>>& meshes, const std::string& path)
{
	const std::string supportedFormat = ".obj";
	if (path.length() > supportedFormat.length() && std::equal(supportedFormat.rbegin(), supportedFormat.rend(), path.rbegin()))
	{
		auto nameStart = path.find_last_of('/') + 1;
		auto extensionLength = supportedFormat.length();
		auto directory = path.substr(0, nameStart);
		auto name = path.substr(nameStart, path.length() - nameStart - extensionLength);

		loadObjAsMesh(meshes, directory, name);

		//meshes.resize(2);
		//loadObjAsMesh(meshes[1], path);

		//printf("mesh");
	}

	return false;
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

	printf(message, elementCount, totalByteSize, totalByteSize / static_cast<float>(elementCount));
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
		printf("Could not allocate vertex memory buffer.");
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

bool Mesh::allocateIndexAttributes(VkMesh& graphicsMesh, const VmaAllocator& vmaAllocator)
{
	size_t totalSize = vectorsizeof(m_indices);

	VkBuffer iBuffer;
	VmaAllocation iMemRange;
	if (!vkinit::MemoryBuffer::allocateBufferAndMemory(iBuffer, iMemRange, vmaAllocator, as_uint32(totalSize), VMA_MEMORY_USAGE_CPU_TO_GPU, VK_BUFFER_USAGE_INDEX_BUFFER_BIT))
	{
		printf("Could not allocate index memory buffer.");
		return false;
	}

	graphicsMesh.iAttributes = MAKEUNQ<IndexAttributes>(iBuffer, iMemRange, VkIndexType::VK_INDEX_TYPE_UINT16);
	graphicsMesh.iCount = as_uint32(m_indices.size());

	// Fill the index buffer
	mapAndCopyBuffer(vmaAllocator, iMemRange, m_indices.data(), m_indices.size() / 3, totalSize,
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
	if (!allocateVertexAttributes(*graphicsMesh, vmaAllocator) ||
		!allocateIndexAttributes(*graphicsMesh, vmaAllocator))
		return false;

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