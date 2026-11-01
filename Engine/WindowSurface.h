#pragma once
#include "Base.h"
#include "Instance.h"

namespace Engine
{
	class WindowSurface
	{
	public:
		using Ptr = std::shared_ptr<WindowSurface>;
		static Ptr create(Instance::Ptr instance, HWND hwnd, HINSTANCE hInstance) 
		{ return std::make_shared<WindowSurface>(instance, hwnd, hInstance); }

		WindowSurface(Instance::Ptr instance, HWND hwnd, HINSTANCE hInstance);

		~WindowSurface();

		[[nodiscard]] auto getSurface() const { return mSurface; }

	private:
		VkSurfaceKHR mSurface{ VK_NULL_HANDLE };
		Instance::Ptr mInstance{ nullptr };
	};
	#pragma warning(pop)
}


