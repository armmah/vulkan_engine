#include "pch.h"
#include "Path.h"

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

bool Path::operator ==(const Path& other) const { return value == other.value; }
const char* Path::c_str() const { return value.c_str(); }
Path::operator const std::string() const { return value; }