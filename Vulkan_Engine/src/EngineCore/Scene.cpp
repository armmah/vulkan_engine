#include "pch.h"
#include "Scene.h"
#include "Color.h"

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
