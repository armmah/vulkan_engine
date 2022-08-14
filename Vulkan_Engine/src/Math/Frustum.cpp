#include "pch.h"
#include "BoundsAABB.h"
#include "Frustum.h"
#include "Camera.h"

Frustum::Frustum(const Camera& cam)
{
	const auto& viewProjMat = cam.getViewProjectionMatrix();
	auto transposedMat = glm::transpose(viewProjMat);

	m_planes[Left] = transposedMat[3] + transposedMat[0];
	m_planes[Right] = transposedMat[3] - transposedMat[0];

	m_planes[Bottom] = transposedMat[3] + transposedMat[1];
	m_planes[Top] = transposedMat[3] - transposedMat[1];

#ifdef GLM_FORCE_DEPTH_ZERO_TO_ONE
	m_planes[Near] = transposedMat[2];
#else
	m_planes[Near] = transposedMat[3] + transposedMat[2];
#endif

	m_planes[Far] = transposedMat[3] - transposedMat[2];
}

// Check if the world space bounds are inside the camera frustum.
bool Frustum::isOnFrustum(const BoundsAABB& bounds) const
{
	return bounds.isOnOrForwardPlane(m_planes[Left]) &&
		bounds.isOnOrForwardPlane(m_planes[Right]) &&
		bounds.isOnOrForwardPlane(m_planes[Top]) &&
		bounds.isOnOrForwardPlane(m_planes[Bottom]) &&
		bounds.isOnOrForwardPlane(m_planes[Far]) &&
		bounds.isOnOrForwardPlane(m_planes[Near]);
}
