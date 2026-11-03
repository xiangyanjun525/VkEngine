#include "DescriptorSet.h"

namespace Engine
{
	DescriptorSet::DescriptorSet(
		const Device::Ptr& device,
		const std::vector<UniformParameter::Ptr>& params,
		const DescriptorSetLayout::Ptr& layout,
		const DescriptorPool::Ptr& pool,
		int frameCount)
	{
		mDevice = device;
		mPool = pool;

		std::vector<VkDescriptorSetLayout> layouts(frameCount, layout->getLayout());
		VkDescriptorSetAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = pool->getPool();
		allocateInfo.descriptorSetCount = frameCount;
		allocateInfo.pSetLayouts = layouts.data();

		mDescriptorSets.resize(frameCount);

		if (vkAllocateDescriptorSets(mDevice->getDevice(), &allocateInfo, mDescriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Error: failed to allocate descriptor sets");
		}

		for (int i = 0; i < frameCount; ++i)
		{
			std::vector<VkWriteDescriptorSet> descriptorSetWrites{};
			for (const auto& param : params)
			{
				VkWriteDescriptorSet descriptorSetWrite{};
				descriptorSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorSetWrite.dstSet = mDescriptorSets[i];
				descriptorSetWrite.dstArrayElement = 0;
				descriptorSetWrite.descriptorType = param->mDescriptorType;
				descriptorSetWrite.descriptorCount = param->mCount;
				descriptorSetWrite.dstBinding = param->mBinding;
				if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				{
					descriptorSetWrite.pBufferInfo = &param->mBuffers[i]->getBufferInfo();
				}

				if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				{
					descriptorSetWrite.pImageInfo = &param->mTexture->getImageInfo();
				}
				descriptorSetWrites.push_back(descriptorSetWrite);
			}
			vkUpdateDescriptorSets(mDevice->getDevice(), static_cast<uint32_t>(descriptorSetWrites.size()), descriptorSetWrites.data(), 0, nullptr);
		}
	}

	DescriptorSet::~DescriptorSet()
	{
		vkFreeDescriptorSets(mDevice->getDevice(), mPool->getPool(), static_cast<uint32_t>(mDescriptorSets.size()), mDescriptorSets.data());
	}
}
