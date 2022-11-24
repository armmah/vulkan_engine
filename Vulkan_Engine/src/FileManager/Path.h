#pragma once
#include "pch.h"

struct Path
{
	std::string value;

	Path();
	Path(std::string&& path);

	std::string getFileDirectory() const;

	std::string getFileName(bool includeExtension) const;

	Path combine(const char* str) const;
	Path combine(const std::string& str) const;
	Path combine(const std::string&& str) const;
	void remove(const std::string& str);
	void removeDirectory(const std::string& str);
	bool matchesExtension(const std::string& ext) const;
	bool fileExists() const;

	bool operator ==(const Path& other) const;
	const char* c_str() const;
	operator const std::string&() const;
};