#pragma once
#include "Base.h"
#include "Device.h"
#include "RenderPass.h"

namespace Engine
{
	class Framebuffer
	{
	public:
		using Ptr = std::shared_ptr<Framebuffer>;
		static Ptr create(
			const Device::Ptr& device,
			const std::vector<VkImageView>& attachments,
			const RenderPass::Ptr& renderPass,
			int width,int height)
		{
			return std::make_shared<Framebuffer>(device, attachments, renderPass, width, height);
		}

		Framebuffer(
			const Device::Ptr& device,
			const std::vector<VkImageView>& attachments,
			const RenderPass::Ptr& renderPass,
			int width, int height);

		~Framebuffer();

		[[nodiscard]] auto getFramebuffer() const { return mFramebuffer; }
	private:
		Device::Ptr mDevice;
		VkFramebuffer mFramebuffer{ VK_NULL_HANDLE };
	};
}

