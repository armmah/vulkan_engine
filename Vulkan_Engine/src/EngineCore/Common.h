#pragma once
#include "pch.h"

#define UNQ std::unique_ptr
#define MAKEUNQ std::make_unique

#define REF std::shared_ptr
#define MAKEREF std::make_shared


#define as_uint32(x) static_cast<uint32_t>(x)

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