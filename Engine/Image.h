#pragma once
#include "Base.h"
#include "Device.h"
#include "CommandPool.h"

namespace Engine
{
	class Image
	{
	public:
		using Ptr = std::shared_ptr<Image>;

		static Ptr create(
			const Device::Ptr& device,
			const int& width,
			const int& height,
			const VkFormat& format,
			const VkImageType& imageType,
			const VkImageTiling& imageTiling,
			const VkImageUsageFlags& usage,
			const VkSampleCountFlagBits& sample,
			const VkMemoryPropertyFlags& properties,
			const VkImageAspectFlags& aspectFlags) {
			return std::make_shared<Image>(
				device,
				width,
				height,
				format,
				imageType,
				imageTiling,
				usage,
				sample,
				properties,
				aspectFlags);
		}

		Image(const Device::Ptr& device,
			const int& width,
			const int& height,
			const VkFormat& format,
			const VkImageType& imageType,
			const VkImageTiling& imageTiling,
			const VkImageUsageFlags& usage,
			const VkSampleCountFlagBits& sample,
			const VkMemoryPropertyFlags& properties,
			const VkImageAspectFlags& aspectFlags);

		~Image();

		[[nodiscard]] auto getImage() const { return mImage; }

		[[nodiscard]] auto getLayout() const { return mLayout; }

		[[nodiscard]] auto getWidth() const { return mWidth; }

		[[nodiscard]] auto getHeight() const { return mHeight; }

		[[nodiscard]] auto getImageView() const { return mImageView; }

		[[nodiscard]] auto getFormat() const { return mFormat; }

		void updateLayout(VkImageLayout layout) { mLayout = layout; }

	private:
		size_t				mWidth;
		size_t				mHeight;
		Device::Ptr			mDevice{ nullptr };
		VkImage				mImage{ VK_NULL_HANDLE };
		VkDeviceMemory		mImageMemory{ VK_NULL_HANDLE };
		VkImageView			mImageView{ VK_NULL_HANDLE };
		VkFormat			mFormat;

		VkImageLayout		mLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
	};
}

