#include "CommandBuffer.h"

namespace Engine
{
	//*---------------------------------------------------------------------*/
	CommandBuffer::CommandBuffer(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool isSecondary)
	{
		mDevice = device;
		mCommandPool = commandPool;

		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.level = isSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandPool = commandPool->getCommandPool();
		allocateInfo.commandBufferCount = 1;
		if (vkAllocateCommandBuffers(mDevice->getDevice(), &allocateInfo, &mCommandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:faield to allocate commandBuffer");
		}
	}
	//*---------------------------------------------------------------------*/
	CommandBuffer::~CommandBuffer()
	{
		vkFreeCommandBuffers(mDevice->getDevice(), mCommandPool->getCommandPool(), 1, &mCommandBuffer);
	}
	//*---------------------------------------------------------------------*/
	void CommandBuffer::begin(VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo& inheritance)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = flags;
		beginInfo.pInheritanceInfo = &inheritance;
		if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:failed to begin command buffer");
		}
	}
	//*---------------------------------------------------------------------*/
	void CommandBuffer::beginRenderPass(const VkRenderPassBeginInfo& renderPassBeginInfo, const VkSubpassContents& subPassContents)
	{
		vkCmdBeginRenderPass(mCommandBuffer, &renderPassBeginInfo, subPassContents);
	}
	//*---------------------------------------------------------------------*/
	void CommandBuffer::bindPipeline(const VkPipeline& pipeline, VkPipelineBindPoint bindPoint)
	{
		vkCmdBindPipeline(mCommandBuffer, bindPoint, pipeline);
	}
	//*---------------------------------------------------------------------*/
	void CommandBuffer::bindVertexBuffer(const std::vector<VkBuffer>& buffers)
	{
		std::vector<VkDeviceSize> offsets(buffers.size(), 0);
		vkCmdBindVertexBuffers(mCommandBuffer, 0, static_cast<uint32_t>(buffers.size()), buffers.data(), offsets.data());
	}
	//*---------------------------------------------------------------------*/
	void CommandBuffer::bindIndexBuffer(const VkBuffer buffer, VkIndexType indexType)
	{
		vkCmdBindIndexBuffer(mCommandBuffer, buffer, 0, indexType);
	}
	//*---------------------------------------------------------------------*/
	void CommandBuffer::bindDescriptorSet(const VkPipelineLayout layout, 
		const VkDescriptorSet& descriptorSet,
		VkPipelineBindPoint bindPoint)
	{
		vkCmdBindDescriptorSets(mCommandBuffer, bindPoint, layout, 0, 1, &descriptorSet, 0, nullptr);
	}
	//*---------------------------------------------------------------------*/
	void CommandBuffer::draw(uint32_t vertexCount)
	{
		vkCmdDraw(mCommandBuffer, vertexCount, 1, 0, 0);
	}
	//*---------------------------------------------------------------------*/
	void CommandBuffer::drawIndex(uint32_t indexCount)
	{
		vkCmdDrawIndexed(mCommandBuffer, indexCount, 1, 0, 0, 0);
	}
	//*---------------------------------------------------------------------*/
	void CommandBuffer::endRenderPass()
	{
		vkCmdEndRenderPass(mCommandBuffer);
	}
	//*---------------------------------------------------------------------*/
	void CommandBuffer::end()
	{
		if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:failed to end command buffer");
		}
	}
	//*---------------------------------------------------------------------*/
	void CommandBuffer::copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, 
		const std::vector<VkBufferCopy>& copyInfos)
	{
		vkCmdCopyBuffer(mCommandBuffer, srcBuffer, dstBuffer, static_cast<uint32_t>(copyInfos.size()), copyInfos.data());
	}
	//*---------------------------------------------------------------------*/
	void CommandBuffer::copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, 
		VkImageLayout dstImageLayout, uint32_t width, uint32_t height)
	{
		VkBufferImageCopy region{};
		region.bufferOffset = 0;

		//为0代表不需要进行padding
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(mCommandBuffer, srcBuffer, dstImage, dstImageLayout, 1, &region);
	}
	//*---------------------------------------------------------------------*/
	void CommandBuffer::submitSync(VkQueue queue, VkFence fence)
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, fence);

		vkQueueWaitIdle(queue);
	}
	//*---------------------------------------------------------------------*/
	void CommandBuffer::transferImageLayout(const VkImageMemoryBarrier& imageMemoryBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
	{
		vkCmdPipelineBarrier(
			mCommandBuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr, //MemoryBarrier
			0, nullptr, //BufferMemoryBarrier
			1, &imageMemoryBarrier);
	}
	//*---------------------------------------------------------------------*/
}