#include "DescriptorPool.h"

namespace Engine
{
	DescriptorPool::DescriptorPool(const Device::Ptr& device)
	{
		mDevice = device;
	}

	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorPool(mDevice->getDevice(), mPool, nullptr);
	}

	void DescriptorPool::build(const std::vector<UniformParameter::Ptr>& params, const int frameCount)
	{
		int uniformBufferCount = 0;
		int textureCount = 0;

		for (const auto& parm : params)
		{
			if (parm->mDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				uniformBufferCount++;
			}
			if (parm->mDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				textureCount++;
			}
		}

		//只添加实际用到的描述符类型，descriptorCount 必须大于 0
		std::vector<VkDescriptorPoolSize> poolSize{};

		if (uniformBufferCount > 0)
		{
			VkDescriptorPoolSize uniformBufferSize{};
			uniformBufferSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uniformBufferSize.descriptorCount = uniformBufferCount * frameCount;
			poolSize.push_back(uniformBufferSize);
		}

		if (textureCount > 0)
		{
			VkDescriptorPoolSize textureSamplerSize{};
			textureSamplerSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			textureSamplerSize.descriptorCount = textureCount * frameCount;
			poolSize.push_back(textureSamplerSize);
		}

		//创建pool
		VkDescriptorPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
		createInfo.pPoolSizes = poolSize.data();
		createInfo.maxSets = static_cast<uint32_t>(frameCount);
		createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		if (vkCreateDescriptorPool(mDevice->getDevice(), &createInfo, nullptr, &mPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Error: failed to create descriptor pool");
		}
	}
}
