#pragma once
#include "pch.h"
#include "FileManager/Directories.h"

struct FileIO
{
	static std::vector<char> readFile(const Path& filename)
	{
		std::ifstream file(filename.c_str(), std::ios::ate | std::ios::binary);

		if (!file.good() || file.fail() || !file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}
};

struct ShaderSource
{
	Path vertexPath,
		fragmentPath;

	ShaderSource(std::string&& path_vertexShaderSource, std::string&& path_fragmentShaderSource)
		: vertexPath(std::move(path_vertexShaderSource)), fragmentPath(std::move(path_fragmentShaderSource)) { }

	ShaderSource(Path&& path_vertexShaderSource, Path&& path_fragmentShaderSource)
		: vertexPath(path_vertexShaderSource), fragmentPath(path_fragmentShaderSource) { }

	std::vector<char> getVertexSource() const { return FileIO::readFile(vertexPath); }
	std::vector<char> getFragmentSource() const { return FileIO::readFile(fragmentPath); }

	static ShaderSource getHardcodedTriangle() 
	{ 
		return ShaderSource(
			Directories::getShaderLibraryPath().combine("triangle.vert.spv"), 
			Directories::getShaderLibraryPath().combine("triangle.frag.spv")
		); 
	}

	static ShaderSource getDefaultShader() 
	{ 
		return ShaderSource(
			Directories::getShaderLibraryPath().combine("simple.vert.spv"),
			Directories::getShaderLibraryPath().combine("simple.frag.spv")
		); 
	}
};