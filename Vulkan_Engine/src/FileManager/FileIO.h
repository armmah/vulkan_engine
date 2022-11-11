#pragma once
#include "pch.h"

struct Path;

struct FileIO
{
	static bool readFile(std::vector<char>& buffer, const Path& filename);
};