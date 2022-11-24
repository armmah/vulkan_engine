#pragma once
#include "pch.h"

struct Path;

struct FileIO
{
	static bool readFile(std::vector<char>& buffer, const Path& filename);

	static bool fileExists(const std::string& path) { return std::filesystem::exists(path); }
	static bool fileExists(const std::string& path, const std::string& supportedFormat)
	{
		return std::filesystem::exists(path) &&
			path.length() > supportedFormat.length() &&
			std::equal(supportedFormat.rbegin(), supportedFormat.rend(), path.rbegin());
	}

	static bool fileExists(const std::string& path, const std::vector<std::string>& supportedFormats)
	{
		if (!std::filesystem::exists(path))
			return false;

		for (const auto& format : supportedFormats)
		{
			if (path.length() > format.length() && std::equal(format.rbegin(), format.rend(), path.rbegin()))
				return true;
		}
		return false;
	}
};