#include "SwapChain.h"
#include "Utils.h"

namespace Engine
{
	//*---------------------------------------------------------------------*/
	SwapChain::SwapChain(
		const Device::Ptr& device,
		const WindowSurface::Ptr& surface,
		int width, int height,
		bool vsync/* = false*/, bool fullscreen /*= false*/)
	{
		mDevice = device;
		mSurface = surface;

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->getPhysicalDevice(), surface->getSurface(), &formatCount, NULL);
		assert(formatCount > 0);
		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->getPhysicalDevice(), surface->getSurface(), &formatCount, surfaceFormats.data());
	
		// We want to get a format that best suits our needs, so we try to get one from a set of preferred formats
	// Initialize the format to the first one returned by the implementation in case we can't find one of the preffered formats
		VkSurfaceFormatKHR selectedFormat = surfaceFormats[0];
		std::vector<VkFormat> preferredImageFormats = {
			VK_FORMAT_B8G8R8A8_UNORM,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_FORMAT_A8B8G8R8_UNORM_PACK32
		};
		for (auto& availableFormat : surfaceFormats) {
			if (std::find(preferredImageFormats.begin(), preferredImageFormats.end(), availableFormat.format) != preferredImageFormats.end()) {
				selectedFormat = availableFormat;
				break;
			}
		}

		mColorFormat = selectedFormat.format;
		mColorSpace = selectedFormat.colorSpace;

		VkSurfaceCapabilitiesKHR surfaceCaps;
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &surfaceCaps));

		VkExtent2D swapchainExtent = {};
		// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
		if (surfaceCaps.currentExtent.width == (uint32_t)-1) {
			// If the surface size is undefined, the size is set to the size of the images requested
			swapchainExtent.width = width;
			swapchainExtent.height = height;
		}
		else {
			// If the surface size is defined, the swap chain size must match
			swapchainExtent = surfaceCaps.currentExtent;
			width = surfaceCaps.currentExtent.width;
			height = surfaceCaps.currentExtent.height;
		}

		// Select a present mode for the swapchain
		uint32_t presentModeCount;
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, nullptr));
		assert(presentModeCount > 0);

		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, presentModes.data()));

		// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
		// This mode waits for the vertical blank ("v-sync")
		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

		// If v-sync is not requested, try to find a mailbox mode
	// It's the lowest latency non-tearing present mode available
		if (!vsync)
		{
			for (size_t i = 0; i < presentModeCount; i++)
			{
				if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
				{
					swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
		}

		// Determine the number of images
		uint32_t desiredNumberOfSwapchainImages = surfaceCaps.minImageCount + 1;
		if ((surfaceCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfaceCaps.maxImageCount)) {
			desiredNumberOfSwapchainImages = surfaceCaps.maxImageCount;
		}

		// Find the transformation of the surface
		VkSurfaceTransformFlagsKHR preTransform;
		if (surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
			// We prefer a non-rotated transform
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else {
			preTransform = surfaceCaps.currentTransform;
		}

		// Find a supported composite alpha format (not all devices support alpha opaque)
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// Simply select the first composite alpha format available
		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& compositeAlphaFlag : compositeAlphaFlags) {
			if (surfaceCaps.supportedCompositeAlpha & compositeAlphaFlag) {
				compositeAlpha = compositeAlphaFlag;
				break;
			};
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mSurface->getSurface();
		createInfo.minImageCount = desiredNumberOfSwapchainImages;
		createInfo.imageFormat = mColorFormat;
		createInfo.imageColorSpace = mColorSpace;
		createInfo.imageExtent = { swapchainExtent.width, swapchainExtent.height };
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
		createInfo.compositeAlpha = compositeAlpha;
		createInfo.presentMode = swapchainPresentMode;
		// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
		createInfo.clipped = VK_TRUE;
		
		// Enable transfer source on swap chain images if supported
		if (surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		// Enable transfer destination on swap chain images if supported
		if (surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		if (vkCreateSwapchainKHR(mDevice->getDevice(), &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swapchain");
		}
	}
	//*---------------------------------------------------------------------*/
	SwapChain::~SwapChain()
	{
		for (auto i = 0; i < mImages.size(); i++) {
			vkDestroyImageView(mDevice->getDevice(), mImageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(mDevice->getDevice(), mSwapChain, nullptr);
	}
	//*---------------------------------------------------------------------*/
	VkResult SwapChain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex)
	{
		return vkAcquireNextImageKHR(mDevice->getDevice(), mSwapChain, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, &imageIndex);
	}
	//*---------------------------------------------------------------------*/
}