#include "Framebuffer.h"

namespace Engine
{
	//*---------------------------------------------------------------------*/
	Framebuffer::Framebuffer(
		const Device::Ptr& device, 
		const std::vector<VkImageView>& attachments, 
		const RenderPass::Ptr& renderPass, 
		int width, int height)
	{
		mDevice = device;

		VkFramebufferCreateInfo frameBufferCreateInfo{};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		frameBufferCreateInfo.pAttachments = attachments.data();
		frameBufferCreateInfo.renderPass = renderPass->getRenderPass();
		frameBufferCreateInfo.width = width;
		frameBufferCreateInfo.height = height;
		frameBufferCreateInfo.layers = 1;

		if (vkCreateFramebuffer(mDevice->getDevice(), &frameBufferCreateInfo, nullptr, &mFramebuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Error: failed to create frameBuffer");
		}
	}
	//*---------------------------------------------------------------------*/
	Framebuffer::~Framebuffer()
	{
		vkDestroyFramebuffer(mDevice->getDevice(), mFramebuffer, nullptr);
	}
	//*---------------------------------------------------------------------*/
}
