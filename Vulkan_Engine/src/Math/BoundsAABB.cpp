#include "pch.h"
#include "BoundsAABB.h"
#include "Plane.h"

// If no bounds are given we inflate the extent to infinity, to be always in the frustrum.
BoundsAABB::BoundsAABB() : center(), extents(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()) {}

BoundsAABB::BoundsAABB(const glm::vec3& min, const glm::vec3& max)
	: center((max + min) * 0.5f), extents(max.x - center.x, max.y - center.y, max.z - center.z) {}

BoundsAABB::BoundsAABB(const glm::vec3& center, float x, float y, float z) : center(center), extents(x, y, z) {}

BoundsAABB BoundsAABB::getTransformed(const glm::mat4& matrix) const
{
#ifndef SAFE_FALLBACK
	BoundsAABB result{};

	for (int i = 0; i < 3; i++)
	{
		result.center[i] = matrix[3][i];
		result.extents[i] = 0.f;

		for (int k = 0; k < 3; k++)
		{
			result.center[i] += matrix[k][i] * center[k];
			result.extents[i] += std::abs(matrix[k][i]) * extents[k];
		}
	}

	return result;
#else
	std::array<glm::vec4, 8> corners = {
		glm::vec4((center - glm::vec3(-extents.x, extents.y, -extents.z)), 1.0f),
		glm::vec4((center - glm::vec3(-extents.x, extents.y, extents.z)), 1.0f),
		glm::vec4((center - glm::vec3(extents.x, extents.y, -extents.z)), 1.0f),
		glm::vec4((center - glm::vec3(extents.x, extents.y, extents.z)), 1.0f),

		glm::vec4((center - glm::vec3(-extents.x, -extents.y, -extents.z)), 1.0f),
		glm::vec4((center - glm::vec3(-extents.x, -extents.y, extents.z)), 1.0f),
		glm::vec4((center - glm::vec3(extents.x, -extents.y, -extents.z)), 1.0f),
		glm::vec4((center - glm::vec3(extents.x, -extents.y, extents.z)), 1.0f),
	};

	auto min = glm::vec3(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
	auto max = glm::vec3(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());

	for (auto& c : corners)
	{
		c = matrix * c;

		min = glm::vec3(std::min(min.x, c.x), std::min(min.y, c.y), std::min(min.z, c.z));
		max = glm::vec3(std::max(max.x, c.x), std::max(max.y, c.y), std::max(max.z, c.z));
	}

	auto bnd = BoundsAABB(min, max);

	auto EPSILON = 0.01f;
	assert(
		glm::length(result.center - bnd.center) < EPSILON &&
		glm::length(result.extents - bnd.extents) < EPSILON
	);

	return bnd;
#endif
}

bool BoundsAABB::isOnOrForwardPlane(const Plane& plane) const
{
	const auto& normal = plane.getNormal();
	// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
	const float r = extents.x * std::abs(normal.x) + extents.y * std::abs(normal.y) + extents.z * std::abs(normal.z);
	
	return -r <= plane.getSignedDistanceToPlane(center);
}
