#pragma once
#include "pch.h"

struct Transform
{
	glm::mat4 localToWorld;

	Transform();
	Transform(const glm::mat4& mat);
	Transform(glm::mat4&& mat);

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& localToWorld;
	}

	bool operator==(const Transform& other) const;
	bool operator!=(const Transform& other) const;
};
