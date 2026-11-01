#include "Buffer.h"
#include "Utils.h"
#include "CommandBuffer.h"
namespace Engine
{
	//*---------------------------------------------------------------------*/
	Buffer::Buffer(const Device::Ptr& device, 
		VkSharingMode shareMode,
		VkDeviceSize size, 
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties)
	{
		mDevice = device;

		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.sharingMode = shareMode;
		createInfo.size = size;
		createInfo.usage = usage;

		if (vkCreateBuffer(mDevice->getDevice(), &createInfo, nullptr, &mBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create buffer");
		}

		VkMemoryRequirements memReq{};
		vkGetBufferMemoryRequirements(mDevice->getDevice(), mBuffer, &memReq);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;

		allocInfo.memoryTypeIndex = Utils::findMemoryType(mDevice, memReq.memoryTypeBits, properties);

		if (vkAllocateMemory(mDevice->getDevice(), &allocInfo, nullptr, &mBufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:failed to allocate buffer memory!");
		}

		if (vkBindBufferMemory(mDevice->getDevice(), mBuffer, mBufferMemory, 0) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:failed to bind buffer memory");
		}

		mBufferInfo.buffer = mBuffer;
		mBufferInfo.offset = 0;
		mBufferInfo.range = size;
	}
	//*---------------------------------------------------------------------*/
	Buffer::~Buffer()
	{
		vkDestroyBuffer(mDevice->getDevice(), mBuffer, nullptr);
		vkFreeMemory(mDevice->getDevice(), mBufferMemory, nullptr);
	}
	//*---------------------------------------------------------------------*/
	void Buffer::updateBufferByMap(void* data, size_t size)
	{
		void* memPtr = nullptr;
		if (vkMapMemory(mDevice->getDevice(), mBufferMemory, 0, size, 0, &memPtr) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:failed to map memory");
		}
		memcpy(memPtr, data, size);
		vkUnmapMemory(mDevice->getDevice(), mBufferMemory);
	}
	//*---------------------------------------------------------------------*/
	void Buffer::updateBufferByStage(void* data, size_t size)
	{
		auto stageBuffer = Buffer::create(mDevice, VK_SHARING_MODE_EXCLUSIVE, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stageBuffer->updateBufferByMap(data, size);

		copyBuffer(stageBuffer->getBuffer(), mBuffer, size);
	}
	//*---------------------------------------------------------------------*/
	void Buffer::copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size)
	{
		auto commandPool = CommandPool::create(mDevice);
		auto commandBuffer = CommandBuffer::create(mDevice, commandPool);

		commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferCopy bufferCopy{};
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;
		bufferCopy.size = size;

		commandBuffer->copyBufferToBuffer(srcBuffer, dstBuffer, { bufferCopy });

		commandBuffer->end();

		commandBuffer->submitSync(mDevice->getGraphicAndCompteQueue(), VK_NULL_HANDLE);
	}
	//*---------------------------------------------------------------------*/

}