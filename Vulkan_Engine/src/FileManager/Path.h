#pragma once
#include "pch.h"

struct Path
{
	std::string value;

	Path() : value() { }
	Path(std::string&& path) : value(std::move(path))
	{
		std::replace(value.begin(), value.end(), '\\', '/');
	}

	std::string getFileDirectory() const
	{
		auto index = value.find_last_of('/');
		return value.substr(0, index + 1);
	}

	std::string getFileName(bool includeExtension) const
	{
		auto extIndex = value.find_last_of('.');
		auto nameIndex = value.find_last_of('/') + 1;

		auto totalSize = value.size();
		auto fullSize = totalSize - nameIndex;

		return value.substr(nameIndex, includeExtension ? fullSize : fullSize - (totalSize - extIndex));
	}

	Path combine(const char* str) const;
	Path combine(const std::string& str) const;
	Path combine(const std::string&& str) const;
	void remove(const std::string& str);
	void removeDirectory(const std::string& str);
	bool matchesExtension(const std::string& ext) const
	{
		return std::equal(ext.rbegin(), ext.rend(), value.rbegin());
	}
	bool fileExists() const { return std::filesystem::exists(value); }

	bool operator ==(const Path& other) const;
	const char* c_str() const;
	operator const std::string&() const;
};