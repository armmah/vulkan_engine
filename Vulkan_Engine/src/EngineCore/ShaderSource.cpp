#include "pch.h"
#include "ShaderSource.h"
#include "FileManager/FileIO.h"

ShaderSource::ShaderSource(std::string&& path_vertexShaderSource, std::string&& path_fragmentShaderSource)
	: vertexPath(std::move(path_vertexShaderSource)), fragmentPath(std::move(path_fragmentShaderSource)) { }

ShaderSource::ShaderSource(Path&& path_vertexShaderSource, Path&& path_fragmentShaderSource)
	: vertexPath(path_vertexShaderSource), fragmentPath(path_fragmentShaderSource) { }

bool ShaderSource::getVertexSource(std::vector<char>& sourcecode) const { return FileIO::readFile(sourcecode, vertexPath); }

bool ShaderSource::getFragmentSource(std::vector<char>& sourcecode) const { return FileIO::readFile(sourcecode, fragmentPath); }

ShaderSource ShaderSource::getDefaultShader()
{
	return ShaderSource(
		Directories::getShaderLibraryPath().combine("simple.vert.spv"),
		Directories::getShaderLibraryPath().combine("simple.frag.spv")
	);
}

ShaderSource ShaderSource::getDepthOnlyShader()
{
	return ShaderSource(
		Directories::getShaderLibraryPath().combine("depthonly.vert.spv"),
		Path()
	);
}

ShaderSource ShaderSource::getDebugQuadShader()
{
	return ShaderSource(
		Directories::getShaderLibraryPath().combine("quad.vert.spv"),
		Directories::getShaderLibraryPath().combine("quad.frag.spv")
	);
}