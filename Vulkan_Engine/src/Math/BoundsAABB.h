#pragma once
#include "pch.h"

struct Plane;
struct BoundsAABB
{
	glm::vec3 center;
	glm::vec3 extents;

	BoundsAABB();
	BoundsAABB(const glm::vec3& min, const glm::vec3& max);
	BoundsAABB(const glm::vec3& center, float x, float y, float z);

	BoundsAABB getTransformed(const glm::mat4& matrix) const;

	bool isOnOrForwardPlane(const Plane& plane) const;
};
