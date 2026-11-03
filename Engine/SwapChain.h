#pragma once

#include "Base.h"
#include "Device.h"
#include "WindowSurface.h"
#include "RenderPass.h"
#include "Image.h"
#include "CommandPool.h"
#include "Framebuffer.h"

namespace Engine
{
	class SwapChain
	{
	public:
		using Ptr = std::shared_ptr<SwapChain>;
		static Ptr create(
			const Device::Ptr& device,
			const WindowSurface::Ptr& surface,
			int width, int height,
			bool vsync = false, bool fullscreen = false)
		{
			return std::make_shared<SwapChain>(
				device, surface, width, height, vsync, fullscreen);
		}

		SwapChain(
			const Device::Ptr& device,
			const WindowSurface::Ptr& surface,
			int width, int height,
			bool vsync = false, bool fullscreen = false);

		~SwapChain();

		VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex);

	public:
		[[nodiscard]] auto getFormat() const { return mColorFormat; }
		[[nodiscard]] auto getImageCount() const { return mImageCount; }
		[[nodiscard]] auto getSwapChain() const { return mSwapChain; }
		[[nodiscard]] const std::vector<VkImage>& getImages() const { return mImages; }
		[[nodiscard]] const std::vector<VkImageView>& getImageViews() const { return mImageViews; }
		[[nodiscard]] auto getExtent() const { return VkExtent2D{ mWidth, mHeight }; }
	private:
		Device::Ptr mDevice{ nullptr };
		WindowSurface::Ptr mSurface{ nullptr };

		VkFormat			mColorFormat{};
		VkColorSpaceKHR		mColorSpace{};
		VkSwapchainKHR		mSwapChain{ VK_NULL_HANDLE };

		std::vector<VkImage>		mImages{};
		std::vector<VkImageView>	mImageViews{};
		uint32_t					mImageCount{ 0 };
		uint32_t					mWidth{ 0 };
		uint32_t					mHeight{ 0 };
	};
}

