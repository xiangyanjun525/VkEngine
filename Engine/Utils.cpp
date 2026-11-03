#include "Utils.h"
#include "CommandBuffer.h"

namespace Engine
{
	//*---------------------------------------------------------------------*/
	Image::Ptr Utils::createDepthImage(
		const Device::Ptr& device,
		const int& width,
		const int& height,
		VkSampleCountFlagBits samples)
	{
		std::vector<VkFormat> formats = {
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT_S8_UINT
		};

		VkFormat resultFormat = findSupportedFormat(
			device, formats,
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

		return Image::create(device, width, height, resultFormat,
			VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			samples,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT);
	}
	//*---------------------------------------------------------------------*/
	Image::Ptr Utils::createRenderTargetImage(
		const Device::Ptr& device,
		const int& width,
		const int& height,
		const VkFormat& format)
		{
		return Image::create(
			device, width, height, format,
			VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			device->getMaxUsableCount(),
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);
		}
	//*---------------------------------------------------------------------*/
	VkFormat Utils::findSupportedFormat(
		const Device::Ptr& device,
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features)
	{
		for (auto format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(device->getPhysicalDevice(), format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}

			if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}
		throw std::runtime_error("Error: failed to find surpported format");
	}
	//*---------------------------------------------------------------------*/
	void Utils::setImageLayout(const Image::Ptr& image,
		const Device::Ptr& device,
		VkImageLayout newLayout,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask,
		VkImageSubresourceRange subResourceRange,
		const CommandPool::Ptr& commandPool)
	{
		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.image = image->getImage();
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.oldLayout = image->getLayout();

		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.subresourceRange = subResourceRange;

		switch (newLayout)
		{
			//如果目标是，将图片转换成为一个复制操作的目标图片/,那么被阻塞的操作一定是写入操作
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
			//如果目标是，将图片转换成为一个适合被作为纹理的格式，那么被阻塞的操作一定是读取
			//如果作为texture,那么来源只能有两种,一种是通过map从cpu拷贝而来，一种是通过stagingbuffer拷贝而来
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		{
			if (imageMemoryBarrier.srcAccessMask == 0)
			{
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		{
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		{
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
		break;
		default:
			break;
		}

		image->updateLayout(newLayout);

		auto commandBuffer = CommandBuffer::create(device, commandPool);
		commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		commandBuffer->transferImageLayout(imageMemoryBarrier, srcStageMask, dstStageMask);
		commandBuffer->end();

		commandBuffer->submitSync(device->getGraphicAndCompteQueue());
	}
	//*---------------------------------------------------------------------*/
	void Utils::fillImageData(const Image::Ptr& image,
		const Device::Ptr& device,
		size_t size, void* pData,
		const CommandPool::Ptr& commandPool)
	{
		assert(pData && size);
		auto stageBuffer = Utils::createStageBuffer(device, VK_SHARING_MODE_EXCLUSIVE, size, pData);

		auto commandBuffer = CommandBuffer::create(device, commandPool);

		commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		commandBuffer->copyBufferToImage(stageBuffer->getBuffer(), image->getImage(),
			image->getLayout(), image->getWidth(), image->getHeight());
		commandBuffer->end();

		commandBuffer->submitSync(device->getGraphicAndCompteQueue());
	}
	//*---------------------------------------------------------------------*/
	uint32_t Utils::findMemoryType(const Device::Ptr& device, 
		uint32_t typeFilter, 
		VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProps{};
		vkGetPhysicalDeviceMemoryProperties(device->getPhysicalDevice(), &memProps);

		//0x001 | 0x100 = 0x101 i = 0,第i个对应类型就是 1 << i 1 i = 1 0x010
		for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && ((memProps.memoryTypes[i].propertyFlags & properties) == properties))
			{
				return i;
			}
		}

		throw std::runtime_error("Error:can not find the property memory type!");
	}
	//*---------------------------------------------------------------------*/
	bool Utils::checkImageHasStencilComponent(const Image::Ptr& imgae)
	{
		return (imgae->getFormat() == VK_FORMAT_D32_SFLOAT_S8_UINT) || 
			(imgae->getFormat() == VK_FORMAT_D24_UNORM_S8_UINT);
	}
	//*---------------------------------------------------------------------*/
	Buffer::Ptr Utils::createVertexBuffer(const Device::Ptr& device, VkSharingMode shareMode, VkDeviceSize size, void* pData)
	{
		auto buffer = Buffer::create(
			device, shareMode, size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		buffer->updateBufferByStage(pData, size);

		return buffer;
	}
	//*---------------------------------------------------------------------*/
	Buffer::Ptr Utils::createUniformBuffer(const Device::Ptr& device, VkSharingMode shareMode, VkDeviceSize size, void* pData)
	{
		auto buffer = Buffer::create(
			device, shareMode, size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (pData)
		{
			buffer->updateBufferByMap(pData, size);
		}
		return buffer;
	}
	//*---------------------------------------------------------------------*/
	Buffer::Ptr Utils::createIndexBuffer(const Device::Ptr& device, VkSharingMode shareMode, VkDeviceSize size, void* pData)
	{
		auto buffer = Buffer::create(
			device, shareMode, size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		buffer->updateBufferByStage(pData, size);
		return buffer;
	}
	//*---------------------------------------------------------------------*/
	Buffer::Ptr Utils::createStageBuffer(const Device::Ptr& device, VkSharingMode shareMode, VkDeviceSize size, void* pData)
	{
		auto buffer = Buffer::create(
			device, shareMode, size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		if (pData != nullptr) {
			buffer->updateBufferByMap(pData, size);
		}

		return buffer;
	}
	//*---------------------------------------------------------------------*/
	VkImageView Utils::createImageView(const Device::Ptr device, VkImage image, 
		VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
	{
		VkImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.format = format;
		viewCreateInfo.image = image;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

		viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = mipLevels;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		VkImageView imageView{ VK_NULL_HANDLE };
		if (vkCreateImageView(device->getDevice(), &viewCreateInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Error: failed to create image view in swapchain");
		}
		return imageView;
	}
	//*---------------------------------------------------------------------*/
	std::vector<char> Utils::readBinary(const std::string& fileName)
	{
		std::ifstream file(fileName.c_str(), std::ios::ate | std::ios::binary | std::ios::in);
		if (!file)
		{
			throw std::runtime_error("Error: failed to open shader file");
		}

		const size_t fileSize = file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}
	//*---------------------------------------------------------------------*/
}