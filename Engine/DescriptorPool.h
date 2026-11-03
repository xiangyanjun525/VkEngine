#pragma once
#include "Base.h"
#include "Device.h"
#include "Description.h"

namespace Engine
{
	class DescriptorPool
	{
	public:
		using Ptr = std::shared_ptr<DescriptorPool>;
		static Ptr create(const Device::Ptr& device) { return std::make_shared<DescriptorPool>(device); }

		DescriptorPool(const Device::Ptr& device);

		~DescriptorPool();

		void build(const std::vector<UniformParameter::Ptr>& params, const int frameCount);

		[[nodiscard]] auto getPool() const { return mPool; }
	private:
		VkDescriptorPool mPool{ VK_NULL_HANDLE };
		Device::Ptr mDevice{ nullptr };
	};
}
