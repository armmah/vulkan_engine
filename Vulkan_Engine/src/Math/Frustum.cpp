#include "pch.h"
#include "BoundsAABB.h"
#include "Frustum.h"
#include "Camera.h"

inline static glm::vec4 getRow(const glm::mat4& matrix, int rowIndex);
inline static glm::vec4 addRows(const glm::mat4& matrix, int rowIndex1, int rowIndex2);
inline static glm::vec4 subtractRows(const glm::mat4& matrix, int rowIndex1, int rowIndex2);

Frustum::Frustum(const Camera& cam)
{
	const auto& viewProjMat = cam.getViewProjectionMatrix();

	// Gribb-Hartmann method of frustum plane extraction from VP matrix - http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
	m_planes[EPlanes::Left]		= addRows(viewProjMat,		3, 0);
	m_planes[EPlanes::Right]	= subtractRows(viewProjMat,	3, 0);

	m_planes[EPlanes::Bottom]	= addRows(viewProjMat,		3, 1);
	m_planes[EPlanes::Top]		= subtractRows(viewProjMat,	3, 1);

#ifdef GLM_FORCE_DEPTH_ZERO_TO_ONE
	m_planes[EPlanes::Near]		= getRow(viewProjMat,		3);
#else
	m_planes[EPlanes::Near]		= addColumns(viewProjMat,	3, 2);
#endif
	m_planes[EPlanes::Far]		= subtractRows(viewProjMat,	3, 2);
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

inline static glm::vec4 getRow(const glm::mat4& matrix, int rowIndex)
{
	glm::vec4 result;

	for (int i = 0; i < 4; ++i)
		result[i] = matrix[i][rowIndex];

	return result;
}

inline static glm::vec4 addRows(const glm::mat4& matrix, int rowIndex1, int rowIndex2)
{
	glm::vec4 result;

	for (int i = 0; i < 4; ++i)
		result[i] = matrix[i][rowIndex1] + matrix[i][rowIndex2];

	return result;
}

inline static glm::vec4 subtractRows(const glm::mat4& matrix, int rowIndex1, int rowIndex2)
{
	glm::vec4 result;

	for (int i = 0; i < 4; ++i)
		result[i] = matrix[i][rowIndex1] - matrix[i][rowIndex2];

	return result;
}