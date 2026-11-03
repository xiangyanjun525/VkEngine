#pragma once

#include "Base.h"
#include "Image.h"
#include "Sampler.h"
#include "Device.h"
#include "CommandPool.h"

namespace Engine
{
	class Texture
	{
	public:
		using Ptr = std::shared_ptr<Texture>;
		static Ptr create(
			const Device::Ptr& device,
			const CommandPool::Ptr& commandPool,
			const std::string& imageFilePath) {
			return std::make_shared<Texture>(device, commandPool, imageFilePath);
		}

		Texture(const Device::Ptr& device, const CommandPool::Ptr& commandPool, const std::string& imageFilePath);

		~Texture();

		[[nodiscard]] auto getImage() const { return mImage; }

		[[nodiscard]] auto getSampler() const { return mSampler; }

		[[nodiscard]] auto& getImageInfo() { return mImageInfo; }
	private:
		Image::Ptr mImage{ nullptr };
		Sampler::Ptr mSampler{ nullptr };
		Device::Ptr mDevice{ nullptr };
		VkDescriptorImageInfo mImageInfo;
	};
}

