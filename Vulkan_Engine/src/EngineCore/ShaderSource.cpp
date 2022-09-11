#include "pch.h"
#include "ShaderSource.h"
#include "FileManager/FileIO.h"

ShaderSource::ShaderSource(std::string&& path_vertexShaderSource, std::string&& path_fragmentShaderSource)
	: vertexPath(std::move(path_vertexShaderSource)), fragmentPath(std::move(path_fragmentShaderSource)) { }

ShaderSource::ShaderSource(Path&& path_vertexShaderSource, Path&& path_fragmentShaderSource)
	: vertexPath(path_vertexShaderSource), fragmentPath(path_fragmentShaderSource) { }

std::vector<char> ShaderSource::getVertexSource() const { return FileIO::readFile(vertexPath); }

std::vector<char> ShaderSource::getFragmentSource() const { return FileIO::readFile(fragmentPath); }

ShaderSource ShaderSource::getHardcodedTriangle()
{
	return ShaderSource(
		Directories::getShaderLibraryPath().combine("triangle.vert.spv"),
		Directories::getShaderLibraryPath().combine("triangle.frag.spv")
	);
}

ShaderSource ShaderSource::getDefaultShader()
{
	return ShaderSource(
		Directories::getShaderLibraryPath().combine("simple.vert.spv"),
		Directories::getShaderLibraryPath().combine("simple.frag.spv")
	);
}
