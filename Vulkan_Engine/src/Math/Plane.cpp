#include "pch.h"
#include "Plane.h"

Plane::Plane() : m_normal(), m_distance() { }

Plane::Plane(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
	glm::vec3 p21 = p2 - p1;
	glm::vec3 p32 = p3 - p2;
	m_normal = glm::cross(p21, p32);
	m_normal = glm::normalize(m_normal);
	m_distance = glm::dot(m_normal, p1);
}

Plane::Plane(const glm::vec3& normal, float distance)
{
	float oneOverLength = 1 / glm::length(normal);
	m_normal = normal * oneOverLength;
	m_distance = distance * oneOverLength;
}

const glm::vec3& Plane::getNormal() const { return m_normal; }
float Plane::getDistance() const { return m_distance; }

float Plane::getSignedDistanceToPlane(const glm::vec3& point) const { return glm::dot(m_normal, point) + m_distance; }

void Plane::operator=(glm::vec4 vector)
{
	m_normal = glm::vec3(vector.x, vector.y, vector.z);

	float oneOverLength = 1 / glm::length(m_normal);
	m_normal *= oneOverLength;
	m_distance = vector.w * oneOverLength;
}