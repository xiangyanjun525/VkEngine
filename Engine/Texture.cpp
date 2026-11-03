#include "Texture.h"
#include "Utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Engine
{
	Texture::Texture(const Device::Ptr& device, const CommandPool::Ptr& commandPool, const std::string& imageFilePath)
	{
		mDevice = device;

		int texWidth, texHeight, texChanels;
		stbi_uc* pixels = stbi_load(imageFilePath.c_str(), &texWidth, &texHeight, &texChanels, STBI_rgb_alpha);
		if (!pixels)
		{
			throw std::runtime_error("Error:failed to load texture");
		}

		int texSize = texWidth * texHeight * 4;

		mImage = Image::create(
			mDevice, texWidth, texHeight,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

		VkImageSubresourceRange region{};
		region.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		region.baseArrayLayer = 0;
		region.layerCount = 1;

		region.baseMipLevel = 0;
		region.levelCount = 1;

		Utils::setImageLayout(
			mImage, 
			mDevice,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			region,
			commandPool
		);

		Utils::fillImageData(mImage, mDevice, texSize, (void*)pixels, commandPool);

		Utils::setImageLayout(
			mImage,
			mDevice,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			region,
			commandPool
		);

		stbi_image_free(pixels);

		mSampler = Sampler::create(mDevice, VK_FILTER_LINEAR, VK_FILTER_LINEAR, 
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

		mImageInfo.imageLayout = mImage->getLayout();
		mImageInfo.imageView = mImage->getImageView();
		mImageInfo.sampler = mSampler->getSampler();
	}

	Texture::~Texture()
	{
	}
}
