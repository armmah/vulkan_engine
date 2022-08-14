#include "pch.h"
#include "BoundsAABB.h"
#include "Plane.h"

// If no bounds are given we inflate the extent to infinity, to be always in the frustrum.
BoundsAABB::BoundsAABB() : center(), extents(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()) {}

BoundsAABB::BoundsAABB(const glm::vec3& min, const glm::vec3& max)
	: center((max + min) * 0.5f), extents(max.x - center.x, max.y - center.y, max.z - center.z) { }

BoundsAABB::BoundsAABB(const glm::vec3& center, float x, float y, float z) : center(center), extents(x, y, z) {}

bool BoundsAABB::isOnOrForwardPlane(const Plane& plane) const
{
	const auto& normal = plane.getNormal();
	// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
	const float r = extents.x * std::abs(normal.x) + extents.y * std::abs(normal.y) + extents.z * std::abs(normal.z);
	
	return -r <= plane.getSignedDistanceToPlane(center);
}
