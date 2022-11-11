#include "pch.h"
#include "FileIO.h"
#include "Path.h"

bool FileIO::readFile(std::vector<char>& buffer, const Path& filename)
{
	std::ifstream file(filename.c_str(), std::ios::ate | std::ios::binary);

	if (!file.good() || file.fail() || !file.is_open())
	{
		return false;
	}

	buffer.clear();
	size_t fileSize = (size_t)file.tellg();
	buffer.resize(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return true;
}
