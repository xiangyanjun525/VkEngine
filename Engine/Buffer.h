#pragma once
#include "Base.h"
#include "Device.h"

namespace Engine
{
	class Buffer
	{
	public:
		using Ptr = std::shared_ptr<Buffer>;
		static Ptr create(
			const Device::Ptr& device,
			VkSharingMode shareMode,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties)
		{
			return std::make_shared<Buffer>(device, shareMode, size, usage, properties);
		}

		Buffer(const Device::Ptr& device, 
			VkSharingMode shareMode,
			VkDeviceSize size, 
			VkBufferUsageFlags usage, 
			VkMemoryPropertyFlags properties);

		~Buffer();

		void updateBufferByMap(void* data, size_t size);

		void updateBufferByStage(void* data, size_t size);

		void copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size);

		[[nodiscard]] auto getBuffer() const { return mBuffer; }

		[[nodiscard]] VkDescriptorBufferInfo& getBufferInfo() { return mBufferInfo; }

	private:
		VkBuffer mBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory mBufferMemory{ VK_NULL_HANDLE };
		Device::Ptr mDevice{ nullptr };
		VkDescriptorBufferInfo mBufferInfo{};
	};
}


