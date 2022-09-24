#include "pch.h"
#include "FileIO.h"
#include "Path.h"

std::vector<char> FileIO::readFile(const Path& filename)
{
	std::ifstream file(filename.c_str(), std::ios::ate | std::ios::binary);

	if (!file.good() || file.fail() || !file.is_open())
	{
		throw std::runtime_error("FileIO: Failed to open file " + filename.value);
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}
