#include "pch.h"
#include "Common.h"
#include "VertexBinding.h"

constexpr size_t tinyobj::IndexHash::cantor(size_t a, size_t b) { return (a + b + 1) * (a + b) / 2 + b; }
constexpr size_t tinyobj::IndexHash::cantor(int a, int b, int c) { return cantor(a, cantor(b, c)); }

size_t tinyobj::IndexHash::operator()(const tinyobj::index_t& k) const
{
	return tinyobj::IndexHash::cantor(k.vertex_index, k.normal_index, k.texcoord_index);
}

bool tinyobj::IndexComparison::operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const
{
	return a.vertex_index == b.vertex_index && a.normal_index == b.normal_index && a.texcoord_index == b.texcoord_index;
}

void Loader::Utility::minVector(glm::vec3& min, const glm::vec3& point)
{
	min.x = std::min(min.x, point.x);
	min.y = std::min(min.y, point.y);
	min.z = std::min(min.z, point.z);
}

void Loader::Utility::maxVector(glm::vec3& max, const glm::vec3& point)
{
	max.x = std::max(max.x, point.x);
	max.y = std::max(max.y, point.y);
	max.z = std::max(max.z, point.z);
}

void Loader::Utility::assertIndex(int index, int vertCount, const char* name)
{
	if (index >= std::numeric_limits<MeshDescriptor::TVertexIndices>::max())
		printf("The index of mesh '%s' exceeded the 16 bit precision limit with id {%i}.\n", name, index);

	if (index < 0 || index >= vertCount)
		printf("The index of mesh '%s' was pointing outside of the vertex array {0 <= %i < %i}.\n", name, index, vertCount);
}