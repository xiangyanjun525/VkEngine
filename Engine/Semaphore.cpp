#include "Semaphore.h"
namespace Engine
{
	Semaphore::Semaphore(const Device::Ptr device)
	{
		mDevice = device;
		VkSemaphoreCreateInfo createInf{};
		createInf.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		if (vkCreateSemaphore(mDevice->getDevice(), &createInf, nullptr, &mSemaphore) != VK_SUCCESS)
		{
			throw std::runtime_error("Error: failed to create semaphore");
		}
	}

	Semaphore::~Semaphore()
	{
		vkDestroySemaphore(mDevice->getDevice(), mSemaphore, nullptr);
	}
}

