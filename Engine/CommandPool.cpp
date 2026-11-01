#include "CommandPool.h"
namespace Engine
{
	//*---------------------------------------------------------------------*/
	CommandPool::CommandPool(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag)
	{
		mDevice = device;
		uint32_t queueFamilyIndex = mDevice->getGraphicAndComputeQueueFamily().value();

		VkCommandPoolCreateInfo	createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = queueFamilyIndex;

		//指令修改的属性,指令池的内存属性
		//VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT:分配出来的CommandBuffer可以单独更新,单独重置
		//VK_COMMAND_POOL_CREATE_TRANSIENT_BIT:每一个CommandBuffer不能单独Reset,必须集体重置， vkResetCommandPool
		createInfo.flags = flag;
		if (vkCreateCommandPool(device->getDevice(), &createInfo, nullptr, &mCommandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:failed to create command pool");
		}
	}
	//*---------------------------------------------------------------------*/
	CommandPool::~CommandPool()
	{
		vkDestroyCommandPool(mDevice->getDevice(), mCommandPool, nullptr);
	}
	//*---------------------------------------------------------------------*/
}