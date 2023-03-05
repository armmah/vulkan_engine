#include "pch.h"
#include "Transform.h"

Transform::Transform() : localToWorld(1.0f) { }

Transform::Transform(const glm::mat4& mat) : localToWorld(mat) { }

Transform::Transform(glm::mat4&& mat) : localToWorld(std::move(mat)) { }

bool Transform::operator==(const Transform& other) const { return localToWorld == other.localToWorld; }
bool Transform::operator!=(const Transform& other) const { return !(*this == other); }