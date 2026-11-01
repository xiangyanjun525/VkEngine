#pragma once
#include "Base.h"
#include "Device.h"

namespace Engine
{
	class Sampler
	{
	public:
		using Ptr = std::shared_ptr<Sampler>;
		static Ptr create(const Device::Ptr device,
			VkFilter magFilter, VkFilter minFilter,
			VkSamplerAddressMode u,
			VkSamplerAddressMode v,
			VkSamplerAddressMode w) 
		{ return std::make_shared<Sampler>(
			device,magFilter,minFilter,
			u,v,w); }

		Sampler(const Device::Ptr device,
			VkFilter magFilter, VkFilter minFilter,
			VkSamplerAddressMode u,
			VkSamplerAddressMode v,
			VkSamplerAddressMode w);

		~Sampler();

		[[nodiscard]] auto getSampler() const { return mSampler; }
	private:
		VkSampler mSampler{ VK_NULL_HANDLE };
		Device::Ptr mDevice{ nullptr };
	};
}


