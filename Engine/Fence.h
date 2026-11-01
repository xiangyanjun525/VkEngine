#pragma once
#include "Base.h"
#include "Device.h"

namespace Engine
{
	class Fence
	{
	public:
		using Ptr = std::shared_ptr<Fence>;
		static Ptr create(const Device::Ptr device, bool signaled = true) { return std::make_shared<Fence>(device, signaled); }

		Fence(const Device::Ptr device, bool signaled = true);

		~Fence();

		//置为非激发态
		void resetFence();

		//调用此函数，如果fence没有被激发，那么阻塞在这里，等待激发
		void block(uint64_t timeout = UINT64_MAX);

		[[nodiscard]] auto getFence() const { return mFence; }
	private:
		VkFence mFence{ VK_NULL_HANDLE };
		Device::Ptr mDevice{ nullptr };
	};
}

