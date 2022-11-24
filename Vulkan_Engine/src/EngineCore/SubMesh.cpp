#include "pch.h"
#include "Mesh.h"

SubMesh::SubMesh() : m_indices(), m_bounds() { }
SubMesh::SubMesh(size_t size) : m_indices(), m_bounds() { m_indices.reserve(size); }
SubMesh::SubMesh(std::vector<MeshDescriptor::TVertexIndices>& indices) : m_indices(std::move(indices)), m_bounds() { }
size_t SubMesh::getIndexCount() const { return m_indices.size(); }

bool SubMesh::operator ==(const SubMesh& other) const
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

bool SubMesh::operator !=(const SubMesh& other) const { return !(*this == other); }