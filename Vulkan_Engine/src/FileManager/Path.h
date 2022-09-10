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

	Path combine(const char* str) const { return Path(value + str); }
	Path combine(const std::string& str) const { return Path(value + str); }
	Path combine(std::string&& str) const { return Path(value + str);	}
	void remove(const std::string& str) 
	{
		auto indexStart = value.find(str.c_str());
		auto indexEnd = indexStart + str.length();

		value = value.substr(0, indexStart) + value.substr(indexEnd, value.length() - indexEnd);
	}
	void removeDirectory(const std::string& str)
	{
		auto indexStart = value.find(str.c_str());
		auto indexEnd = indexStart + str.length();

		if (indexStart < 0 && indexEnd < 0)
			return;

		if (indexEnd < value.length() && value[indexEnd] == '/')
			indexEnd += 1;

		value = value.substr(0, indexStart) + value.substr(indexEnd, value.length() - indexEnd);
	}

	bool operator ==(const Path& other) const { return value == other.value; }
	const char* c_str() const { return value.c_str(); }
	operator const std::string() const { return value; }
};