#include "pch.h"
#include "FileManager/Path.h"

namespace Loader
{
	struct ModelLoaderOptions
	{
		Path filePath;
		float sizeModifier;

		ModelLoaderOptions(Path&& path, float scale) : filePath(std::move(path)), sizeModifier(scale) { }
		ModelLoaderOptions(std::string&& path, float scale) : filePath(std::move(path)), sizeModifier(scale) { }
	};
}