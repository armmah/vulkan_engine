#include "pch.h"
#include "Mesh.h"
#include "Color.h"
#include <glm/ext/quaternion_geometric.hpp>

Mesh Mesh::getPrimitiveQuad()
{
	std::vector<MeshDescriptor::TVertexPosition> positions;
	std::vector<MeshDescriptor::TVertexUV> uvs;
	std::vector<MeshDescriptor::TVertexNormal> normals;
	std::vector<MeshDescriptor::TVertexColor> colors;
	std::vector<MeshDescriptor::TVertexIndices> indices;

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

	return Mesh(positions, uvs, normals, colors, SubMesh(indices));
}

void Mesh::makeFace(glm::vec3 pivot, glm::vec3 up, glm::vec3 right, uint16_t firstIndex)
{
	m_positions.push_back(pivot + up - right);
	m_positions.push_back(pivot + up + right);
	m_positions.push_back(pivot - up - right);
	m_positions.push_back(pivot - up + right);

	m_uvs.push_back({ 0.0, 1.0 });
	m_uvs.push_back({ 1.0, 1.0 });
	m_uvs.push_back({ 0.0, 0.0 });
	m_uvs.push_back({ 1.0, 0.0 });

	auto normal = glm::normalize(pivot);
	m_normals.push_back(normal);
	m_normals.push_back(normal);
	m_normals.push_back(normal);
	m_normals.push_back(normal);

	//m_colors.push_back(Color::red().v4());
	//m_colors.push_back(Color::green().v4());
	//m_colors.push_back(Color::green().v4());
	//m_colors.push_back(Color::blue().v4());

	uint16_t indices[6] {
		0, 1, 2,
		2, 1, 3
	};
	m_submeshes.resize(1);
	auto& submesh = m_submeshes[0];

	if (normal.x < 0 || normal.y < 0 || normal.z < 0)
	{
		for (int i = 0; i < 6; i += 1)
			submesh.m_indices.push_back(firstIndex + indices[i]);
	}
	else
	{
		for (int i = 5; i >= 0; i -= 1)
			submesh.m_indices.push_back(firstIndex + indices[i]);
	}
}

Mesh Mesh::getPrimitiveCube()
{
	constexpr uint16_t vertPerFace = 4u;
	constexpr uint16_t indexPerFace = 6u;
	constexpr uint16_t numOfFaces = 6u;
	Mesh mesh(vertPerFace * numOfFaces, indexPerFace * numOfFaces);

	const glm::vec3 right(0.5f, 0.f, 0.f);
	const glm::vec3 up(0.f, 0.5f, 0.f);
	const glm::vec3 forward(0.f, 0.f, 0.5f);

	mesh.makeFace(right, up, -forward, vertPerFace * 0);
	mesh.makeFace(-right, up, -forward, vertPerFace * 1);
	mesh.makeFace(up, right, forward, vertPerFace * 2);
	mesh.makeFace(-up, right, forward, vertPerFace * 3);
	mesh.makeFace(forward, up, right, vertPerFace * 4);
	mesh.makeFace(-forward, up, right, vertPerFace * 5);

	mesh.updateMetaData();
	return mesh;
}

Mesh Mesh::getPrimitiveTriangle()
{
	std::vector<MeshDescriptor::TVertexPosition> positions;
	std::vector<MeshDescriptor::TVertexUV> uvs;
	std::vector<MeshDescriptor::TVertexNormal> normals;
	std::vector<MeshDescriptor::TVertexColor> colors;
	std::vector<MeshDescriptor::TVertexIndices> indices;

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