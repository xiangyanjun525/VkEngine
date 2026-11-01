#include "Sampler.h"

namespace Engine
{
	Sampler::Sampler(const Device::Ptr device, 
		VkFilter magFilter, VkFilter minFilter,
		VkSamplerAddressMode u, 
		VkSamplerAddressMode v,
		VkSamplerAddressMode w)
	{
		mDevice = device;

		VkSamplerCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.magFilter = magFilter;
		createInfo.minFilter = minFilter;
		createInfo.addressModeU = u;
		createInfo.addressModeV = v;
		createInfo.addressModeW = w;

		createInfo.anisotropyEnable = VK_TRUE;
		createInfo.maxAnisotropy = 16;

		createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

		//我们是否使用非归一化的uvw坐标体系
		createInfo.unnormalizedCoordinates = VK_FALSE;

		//采样得到的样本值需要跟一个值进行比较,比较的方式也可以设置，比较通过了测试，才采样，否则不采用
		createInfo.compareEnable = VK_FALSE;
		createInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		createInfo.mipLodBias = 0.0f;
		createInfo.minLod = 0.0f;
		createInfo.maxLod = 0.0f;

		if (vkCreateSampler(mDevice->getDevice(), &createInfo, nullptr, &mSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Error: failed to create sampler");
		}
	}

	Sampler::~Sampler()
	{
		vkDestroySampler(mDevice->getDevice(), mSampler, nullptr);
	}
}
