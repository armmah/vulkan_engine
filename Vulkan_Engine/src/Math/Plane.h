#pragma once
#include "pch.h"

struct Plane
{
	Plane();
	Plane(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
	Plane(const glm::vec3& normal, float distance);

	const glm::vec3& getNormal() const;
	float getDistance() const;

	float getSignedDistanceToPlane(const glm::vec3& point) const;

	void operator=(glm::vec4 vector);

private:
	glm::vec3 m_normal;
	float m_distance;
};