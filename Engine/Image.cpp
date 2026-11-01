#include "Image.h"
#include "Utils.h"

namespace Engine
{
	//*---------------------------------------------------------------------*/
	Image::Image(
		const Device::Ptr& device,
		const int& width,
		const int& height,
		const VkFormat& format,
		const VkImageType& imageType,
		const VkImageTiling& imageTiling,
		const VkImageUsageFlags& usage,
		const VkSampleCountFlagBits& sample,
		const VkMemoryPropertyFlags& properties,
		const VkImageAspectFlags& aspectFlags)
	{
		mDevice = device;
		mLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		mWidth = width;
		mHeight = height;
		mFormat = format;

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.format = format;
		imageCreateInfo.imageType = imageType;
		imageCreateInfo.tiling = imageTiling;
		imageCreateInfo.usage = usage; //color, depth?
		imageCreateInfo.samples = sample;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(mDevice->getDevice(), &imageCreateInfo, nullptr, &mImage) != VK_SUCCESS)
		{
			throw std::runtime_error("Error: failed to create image");
		}

		//分配内存空间
		VkMemoryRequirements memReq{};
		vkGetImageMemoryRequirements(mDevice->getDevice(), mImage, &memReq);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;

		allocInfo.memoryTypeIndex = Utils::findMemoryType(mDevice, memReq.memoryTypeBits, properties);

		if (vkAllocateMemory(mDevice->getDevice(), &allocInfo, nullptr, &mImageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:failed to allocate image memory!");
		}

		vkBindImageMemory(mDevice->getDevice(), mImage, mImageMemory, 0);

		mImageView = Utils::createImageView(mDevice, mImage, mFormat, aspectFlags,1);
	}
	//*---------------------------------------------------------------------*/
	Image::~Image()
	{
		vkDestroyImage(mDevice->getDevice(), mImage, nullptr);
		vkFreeMemory(mDevice->getDevice(), mImageMemory, nullptr);
		vkDestroyImageView(mDevice->getDevice(), mImageView, nullptr);
	}
	//*---------------------------------------------------------------------*/
}
