#pragma once
#include "pch.h"
#include "FileManager/Directories.h"

struct ShaderSource
{
	Path vertexPath,
		fragmentPath;

	ShaderSource(std::string&& path_vertexShaderSource, std::string&& path_fragmentShaderSource);
	ShaderSource(Path&& path_vertexShaderSource, Path&& path_fragmentShaderSource);

	bool getVertexSource(std::vector<char>&) const;
	bool getFragmentSource(std::vector<char>&) const;

	static ShaderSource getDefaultShader();
	static ShaderSource getDepthOnlyShader();
};