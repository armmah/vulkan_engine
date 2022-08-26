#pragma once
#include "pch.h"

#define UNQ std::unique_ptr
#define MAKEUNQ std::make_unique

#define REF std::shared_ptr
#define MAKEREF std::make_shared


#define as_uint32(x) static_cast<uint32_t>(x)
#define as_int32(x) static_cast<int32_t>(x)

constexpr static uint32_t SWAPCHAIN_IMAGE_COUNT = 3u;

template<typename T>
static T bitFlagAppend(T state, T flag)
{
	return static_cast<T>(state | 1 << flag);
}

template<typename T>
static T bitFlagAppendIf(T state, bool condition, T flag)
{
	return static_cast<T>(state | (condition << flag));
}

template<typename T>
static bool bitFlagPresent(T state, T flag)
{
	return (state & (1 << flag)) != 0;
}

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