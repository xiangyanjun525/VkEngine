#include "WindowSurface.h"

namespace Engine
{
	//*---------------------------------------------------------------------*/
	WindowSurface::WindowSurface(Instance::Ptr instance, HWND hwnd, HINSTANCE hInstance)
	{
		mInstance = instance;
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hinstance = hInstance;
		surfaceCreateInfo.hwnd = hwnd;
		if (vkCreateWin32SurfaceKHR(instance->getInstance(), &surfaceCreateInfo, nullptr, &mSurface))
		{
			throw std::runtime_error("Error: failed to create window surface");
		}
	}
	//*---------------------------------------------------------------------*/
	WindowSurface::~WindowSurface()
	{
		vkDestroySurfaceKHR(mInstance->getInstance(), mSurface, nullptr);
	}
	//*---------------------------------------------------------------------*/
}