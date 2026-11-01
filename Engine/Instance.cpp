#include "Instance.h"

namespace Engine
{
	//*---------------------------------------------------------------------*/
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageServerity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pMessageData,
		void* pUserData) {
		std::cout << "ValidationLayer: " << pMessageData->pMessage << std::endl;

		return VK_FALSE;
	}
	//*---------------------------------------------------------------------*/
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* debugMessager) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, debugMessager);

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
	//*---------------------------------------------------------------------*/
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessager,
		const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
			func(instance, debugMessager, pAllocator);

	}
	//*---------------------------------------------------------------------*/
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
		appInfo.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo instCreateInfo = {};
		instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instCreateInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		instCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instCreateInfo.ppEnabledExtensionNames = extensions.data();

		if (mEnableValidationLayer)
		{
			instCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			instCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			instCreateInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&instCreateInfo, nullptr, &mInstance) != VK_SUCCESS)
		{
			throw std::runtime_error("Error: failed to create instance");
		}

		setupDebugger();
	}
	//*---------------------------------------------------------------------*/
	Instance::~Instance()
	{
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugger, nullptr);
		vkDestroyInstance(mInstance, nullptr);
	}
	//*---------------------------------------------------------------------*/
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
	//*---------------------------------------------------------------------*/
	std::vector<const char*> Instance::getRequiredExtensions()
	{
		std::vector<const char*> ret;
		ret.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		ret.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		if (mEnableValidationLayer)
			ret.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		return ret;
	}
	//*---------------------------------------------------------------------*/
	bool Instance::checkValidationLayerSupport()
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const auto& layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProp : availableLayers) {
				if (std::strcmp(layerName, layerProp.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}
	//*---------------------------------------------------------------------*/
	void Instance::setupDebugger()
	{
		if (!mEnableValidationLayer)
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		createInfo.pfnUserCallback = debugCallBack;
		createInfo.pUserData = nullptr;

		if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugger) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:failed to create debugger");
		}
	}
	//*---------------------------------------------------------------------*/
}
