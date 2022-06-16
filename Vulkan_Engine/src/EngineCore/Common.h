#pragma once

#include "pch.h"

#define UNQ std::unique_ptr
#define MAKEUNQ std::make_unique

#define REF std::shared_ptr
#define MAKEREF std::make_shared


#define as_uint32(x) static_cast<uint32_t>(x)

constexpr static uint32_t SWAPCHAIN_IMAGE_COUNT = 3u;
