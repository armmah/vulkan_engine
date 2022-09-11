#pragma once
#include "pch.h"

struct Path;

struct FileIO
{
	static std::vector<char> readFile(const Path& filename);
};