#pragma once

#include "glm/ext/vector_float4.hpp"

struct Color
{
public:
	Color(glm::vec4 normalizedColor) : normalized(normalizedColor) { }
	Color(float r, float g, float b) : normalized(glm::vec4(r, g, b, 1.0f)) { }
	Color(float r, float g, float b, float a) : normalized(glm::vec4(r, g, b, a)) { }

	glm::vec4 v4() const { return normalized; }

	static Color white() { return Color(1.0f, 1.0f, 1.0f); }
	static Color black() { return Color(0.0f, 0.0f, 0.0f); }
	static Color red() { return Color(1.0f, 0.0f, 0.0f); }
	static Color green() { return Color(0.0f, 1.0f, 0.0f); }
	static Color blue() { return Color(0.0f, 0.0f, 1.0f); }

	operator glm::vec4& () { return normalized; }
	operator glm::vec4() const { return normalized; }

private:
	glm::vec4 normalized;
};