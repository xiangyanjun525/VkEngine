#include "Instance.h"

namespace Engine
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageServerity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pMessageData,
		void* pUserData) {
		std::cout << "ValidationLayer: " << pMessageData->pMessage << std::endl;

		return VK_FALSE;
	}

	//辅助函数
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* debugMessager) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, debugMessager);

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessager,
		const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
			func(instance, debugMessager, pAllocator);

	}

	Instance::Instance(bool enableValidationLayer)
	{
		mEnableValidationLayer = enableValidationLayer;

		printAvaliableExtensions();

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "XYJEngine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "NO ENGINE";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo instCreateInfo = {};
		instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instCreateInfo.pApplicationInfo = &appInfo;
	}

	Instance::~Instance()
	{
	}

	void Instance::printAvaliableExtensions()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		std::cout << "Available extensions:" << std::endl;
		for (const auto& extension : extensions)
		{
			std::cout << extension.extensionName << std::endl;
		}
	}
}
