#pragma once
#include "pch.h"

struct FileIO
{
	static std::vector<char> readFile(const std::string& filename)
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
	std::string vertexPath,
		fragmentPath;

	ShaderSource(std::string path_vertexShaderSource, std::string path_fragmentShaderSource)
		: vertexPath(path_vertexShaderSource), fragmentPath(path_fragmentShaderSource) { }

	std::vector<char> getVertexSource() const { return FileIO::readFile(vertexPath); }
	std::vector<char> getFragmentSource() const { return FileIO::readFile(fragmentPath); }

	static ShaderSource getHardcodedTriangle() { return ShaderSource("C:/Git/Vulkan_Engine/Shaders/outputSPV/triangle.vert.spv", "C:/Git/Vulkan_Engine/Shaders/outputSPV/triangle.frag.spv"); }
	static ShaderSource getDefaultShader() { return ShaderSource("C:/Git/Vulkan_Engine/Shaders/outputSPV/simple.vert.spv", "C:/Git/Vulkan_Engine/Shaders/outputSPV/simple.frag.spv"); }
};