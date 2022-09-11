#pragma once
#include "pch.h"
#include "FileManager/Directories.h"

struct ShaderSource
{
	Path vertexPath,
		fragmentPath;

	ShaderSource(std::string&& path_vertexShaderSource, std::string&& path_fragmentShaderSource);
	ShaderSource(Path&& path_vertexShaderSource, Path&& path_fragmentShaderSource);

	std::vector<char> getVertexSource() const;
	std::vector<char> getFragmentSource() const;

	static ShaderSource getHardcodedTriangle();
	static ShaderSource getDefaultShader();
};