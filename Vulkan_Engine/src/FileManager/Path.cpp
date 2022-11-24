#include "pch.h"
#include "Path.h"

Path::Path() : value() { }

Path::Path(std::string&& path) : value(std::move(path))
{
	std::replace(value.begin(), value.end(), '\\', '/');
}

std::string Path::getFileDirectory() const
{
	auto index = value.find_last_of('/');
	return value.substr(0, index + 1);
}

std::string Path::getFileName(bool includeExtension) const
{
	auto extIndex = value.find_last_of('.');
	auto nameIndex = value.find_last_of('/') + 1;

	auto totalSize = value.size();
	auto fullSize = totalSize - nameIndex;

	return value.substr(nameIndex, includeExtension ? fullSize : fullSize - (totalSize - extIndex));
}

Path Path::combine(const char* str) const { return Path(value + str); }
Path Path::combine(const std::string& str) const { return Path(value + str); }
Path Path::combine(const std::string&& str) const { return Path(value + str); }

void Path::remove(const std::string& str)
{
	auto indexStart = value.find(str.c_str());
	auto indexEnd = indexStart + str.length();

	value = value.substr(0, indexStart) + value.substr(indexEnd, value.length() - indexEnd);
}

void Path::removeDirectory(const std::string& str)
{
	auto indexStart = value.find(str.c_str());
	auto indexEnd = indexStart + str.length();

	if (indexStart < 0 && indexEnd < 0)
		return;

	if (indexEnd < value.length() && value[indexEnd] == '/')
		indexEnd += 1;

	value = value.substr(0, indexStart) + value.substr(indexEnd, value.length() - indexEnd);
}

bool Path::matchesExtension(const std::string& ext) const
{
	return std::equal(ext.rbegin(), ext.rend(), value.rbegin());
}

bool Path::fileExists() const { return std::filesystem::exists(value); }

bool Path::operator ==(const Path& other) const { return value == other.value; }
const char* Path::c_str() const { return value.c_str(); }
Path::operator const std::string&() const { return value; }