#pragma once
#include "Base.h"
#include "Device.h"
#include "CommandPool.h"

namespace Engine
{
	class CommandBuffer
	{
	public:
		using Ptr = std::shared_ptr<CommandBuffer>;
		static Ptr create(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool isSecondary = false) {
			return std::make_shared<CommandBuffer>(device, commandPool, isSecondary);
		}

		CommandBuffer(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool isSecondary = false);
		~CommandBuffer();

		void begin(VkCommandBufferUsageFlags flags = 0, const VkCommandBufferInheritanceInfo& inheritance = {});

		void beginRenderPass(const VkRenderPassBeginInfo& renderPassBeginInfo, const VkSubpassContents& subPassContents = VK_SUBPASS_CONTENTS_INLINE);

		void bindPipeline(const VkPipeline& pipeline, VkPipelineBindPoint bindPoint);

		void bindVertexBuffer(const std::vector<VkBuffer>& buffers);

		void bindIndexBuffer(const VkBuffer buffer, VkIndexType indexType);

		void bindDescriptorSet(const VkPipelineLayout layout, 
			const VkDescriptorSet& descriptorSet,
			VkPipelineBindPoint bindPoint);

		void draw(uint32_t vertexCount);

		void drawIndex(uint32_t indexCount);

		void endRenderPass();

		void end();

		void copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const std::vector<VkBufferCopy>& copyInfos);

		void copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t width, uint32_t height);

		void submitSync(VkQueue queue, VkFence fence = VK_NULL_HANDLE);

		void transferImageLayout(const VkImageMemoryBarrier& imageMemoryBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);

		[[nodiscard]] auto getCommandBuffer() const { return mCommandBuffer; }
	private:
		VkCommandBuffer mCommandBuffer{ VK_NULL_HANDLE };
		Device::Ptr mDevice{ nullptr };
		CommandPool::Ptr mCommandPool{ nullptr };
	};
}

