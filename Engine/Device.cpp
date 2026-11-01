#include "Device.h"

namespace Engine
{
	//*---------------------------------------------------------------------*/
	Device::Device(Instance::Ptr instance, WindowSurface::Ptr surface)
	{
		mInstance = instance;
		mSurface = surface;
		pickPhysicalDevice();
		initQueueFamilies(mPhysicalDevice);
		createLogicalDevice();
	}
	//*---------------------------------------------------------------------*/
	Device::~Device()
	{
		vkDestroyDevice(mDevice, nullptr);
	}
	//*---------------------------------------------------------------------*/
	void Device::pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(mInstance->getInstance(), &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw std::runtime_error("Error: failed to enumeratePhysicalDevice");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(mInstance->getInstance(), &deviceCount, devices.data());

		std::multimap<int, VkPhysicalDevice> candidates;
		for (const auto& device : devices)
		{
			int score = rateDevice(device);
			candidates.insert(std::make_pair(score, device));
		}

		if (candidates.rbegin()->first > 0 && isDeviceSuitable(candidates.rbegin()->second))
		{
			mPhysicalDevice = candidates.rbegin()->second;
		}
		if (mPhysicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Error: failed to get physical device");
		}
	}
	//*---------------------------------------------------------------------*/
	int Device::rateDevice(VkPhysicalDevice device)
	{
		int score = 0;

		//设备名称 类型  支持vulkan的版本
		VkPhysicalDeviceProperties deviceProp;
		vkGetPhysicalDeviceProperties(device, &deviceProp);

		//纹理压缩 浮点数运算特性 多视口渲染
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		if (deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		score += deviceProp.limits.maxImageDimension2D;

		if (!deviceFeatures.geometryShader)
		{
			return 0;
		}
		return score;
	}
	//*---------------------------------------------------------------------*/
	bool Device::isDeviceSuitable(VkPhysicalDevice device)
	{
		//设备名称 类型  支持vulkan的版本
		VkPhysicalDeviceProperties deviceProp;
		vkGetPhysicalDeviceProperties(device, &deviceProp);

		//纹理压缩 浮点数运算特性 多视口渲染
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		return deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader && deviceFeatures.samplerAnisotropy;
	}
	//*---------------------------------------------------------------------*/
	void Device::initQueueFamilies(VkPhysicalDevice device)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueCount > 0 && 
				((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
					(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)))
			{
				mGraphicAndComputQueueFamily = i;
			}

			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface->getSurface(), &presentSupport);
			if (presentSupport)
			{
				mPresentQueueFamily = i;
			}

			if (isQueueFamilyComplete())
			{
				break;
			}
			i++;
		}
	}
	//*---------------------------------------------------------------------*/
	bool Device::isQueueFamilyComplete()
	{
		return (mGraphicAndComputQueueFamily.has_value() && mPresentQueueFamily.has_value());
	}
	//*---------------------------------------------------------------------*/
	void Device::createLogicalDevice()
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> queueFamilies = { mGraphicAndComputQueueFamily.value(), mPresentQueueFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : queueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE;

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceRequiredExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceRequiredExtensions.data();

		//Layer层
		if (mInstance->getEnableValidationLayer())
		{
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		}
		else
		{
			deviceCreateInfo.enabledLayerCount = 0;
		}
		if (vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice) != VK_SUCCESS)
		{
			throw std::runtime_error("Error: failed to create logical device");
		}

		vkGetDeviceQueue(mDevice, mGraphicAndComputQueueFamily.value(), 0, &mGraphicAndComputQueue);
		vkGetDeviceQueue(mDevice, mPresentQueueFamily.value(), 0, &mPresentQueue);
	}
	//*---------------------------------------------------------------------*/
	VkSampleCountFlagBits Device::getMaxUsableCount()
	{
		VkPhysicalDeviceProperties props{};
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &props);

		VkSampleCountFlags counts = std::min(
			props.limits.framebufferColorSampleCounts,
			props.limits.framebufferDepthSampleCounts);

		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}
	//*---------------------------------------------------------------------*/
}
