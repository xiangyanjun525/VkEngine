#pragma once

#pragma warning(disable: 4251)
#pragma warning(push)

#define ENGINE_API

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <optional>
#include <set>
#include <fstream>
#include <array>
#include <Windows.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
};

#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cout << "Fatal : VkResult is \"" <<  "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
		assert(res == VK_SUCCESS);																		\
	}																									\
}
