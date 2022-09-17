#pragma once
#include <set>
#include <array>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include <memory>
#include <filesystem>
#include <fstream>

#include <string>
#include <iostream>
#include <chrono>

#include <stdexcept>
#include <cassert>
#include <assert.h>

#include <optional>
#include <functional>

#include <codeanalysis\warnings.h>
#pragma warning(push, 0)
#pragma warning ( disable : ALL_CPPCORECHECK_WARNINGS )
#include "boost/archive/binary_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"

#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/epsilon.hpp>

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

#include "SDL.h"
#include "SDL_vulkan.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_vulkan.h"

#include "stb_image.h"
#include "tiny_obj_loader.h"
#pragma warning(pop)