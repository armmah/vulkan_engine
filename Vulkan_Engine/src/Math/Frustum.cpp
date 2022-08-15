#include "pch.h"
#include "BoundsAABB.h"
#include "Frustum.h"
#include "Camera.h"

Frustum::Frustum(const Camera& cam)
{
	const auto& viewProjMat = cam.getViewProjectionMatrix();
	auto transposedMat = glm::transpose(viewProjMat);

	m_planes[EPlanes::Left] =	transposedMat[3] + transposedMat[0];
	m_planes[EPlanes::Right] =	transposedMat[3] - transposedMat[0];

	m_planes[EPlanes::Bottom] = transposedMat[3] + transposedMat[1];
	m_planes[EPlanes::Top] =	transposedMat[3] - transposedMat[1];

#ifdef GLM_FORCE_DEPTH_ZERO_TO_ONE
	m_planes[EPlanes::Near] =	transposedMat[2];
#else
	m_planes[EPlanes::Near] =	transposedMat[3] + transposedMat[2];
#endif

	m_planes[EPlanes::Far] =	transposedMat[3] - transposedMat[2];
}

// Check if the world space bounds are inside the camera frustum.
bool Frustum::isOnFrustum(const BoundsAABB& bounds) const
{
	return bounds.isOnOrForwardPlane(m_planes[EPlanes::Left]) &&
		bounds.isOnOrForwardPlane(m_planes[EPlanes::Right]) &&
		bounds.isOnOrForwardPlane(m_planes[EPlanes::Top]) &&
		bounds.isOnOrForwardPlane(m_planes[EPlanes::Bottom]) &&
		bounds.isOnOrForwardPlane(m_planes[EPlanes::Far]) &&
		bounds.isOnOrForwardPlane(m_planes[EPlanes::Near]);
}
