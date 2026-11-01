#pragma once
#include "Base.h"
#include "Device.h"
#include "CommandPool.h"
#include "Image.h"
#include "Buffer.h"

namespace Engine
{
	class Utils
	{
	public:
		static Image::Ptr createDepthImage(
			const Device::Ptr& device,
			const int& width,
			const int& height,
			VkSampleCountFlagBits samples);


		static Image::Ptr createRenderTargetImage(
			const Device::Ptr& device,
			const int& width,
			const int& height,
			const VkFormat& format);

		static VkFormat findSupportedFormat(
			const Device::Ptr& device,
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features);

		static void setImageLayout(const Image::Ptr& image,
			const Device::Ptr& device,
			VkImageLayout newLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subResourceRange,
			const CommandPool::Ptr& commandPool);

		static void fillImageData(const Image::Ptr& image,
			const Device::Ptr& device,
			size_t size, void* pData, 
			const CommandPool::Ptr& commandPool);

		static uint32_t findMemoryType(const Device::Ptr& device, 
			uint32_t typeFilter, 
			VkMemoryPropertyFlags properties);

		static bool checkImageHasStencilComponent(const Image::Ptr& imgae);

		static Buffer::Ptr createVertexBuffer(const Device::Ptr& device, VkSharingMode shareMode, VkDeviceSize size, void* pData);

		static Buffer::Ptr createIndexBuffer(const Device::Ptr& device, VkSharingMode shareMode, VkDeviceSize size, void* pData);

		static Buffer::Ptr createUniformBuffer(const Device::Ptr& device, VkSharingMode shareMode, VkDeviceSize size, void* pData = nullptr);

		static Buffer::Ptr createStageBuffer(const Device::Ptr& device, VkSharingMode shareMode, VkDeviceSize size, void* pData = nullptr);
	
		static VkImageView createImageView(const Device::Ptr device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels = 1);
		
		static std::vector<char> readBinary(const std::string& fileName);
	};
}

