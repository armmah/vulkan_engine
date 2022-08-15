#pragma once
#include "pch.h"
#include "Plane.h"

class Camera;
struct BoundsAABB;

struct Frustum
{
	Frustum(const Camera& cam);

	// Check if the world space bounds are inside the camera frustum.
	bool isOnFrustum(const BoundsAABB& bounds) const;

private:
	enum EPlanes
	{
		Left = 0,
		Right = 1,
		Bottom = 2,
		Top = 3,
		Near = 4,
		Far = 5,

		Count = 6
	};

	std::array<Plane, EPlanes::Count> m_planes;
};