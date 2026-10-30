#pragma once

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <optional>
#include <set>
#include <fstream>
#include <array>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
