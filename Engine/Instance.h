#pragma once
#include "Base.h"

namespace Engine
{
	class ENGINE_API Instance
	{
	public:
		using Ptr = std::shared_ptr<Instance>;
		static Ptr create(bool enableValidationLayer) { return std::make_shared<Instance>(enableValidationLayer); }

		Instance(bool enableValidationLayer);

		~Instance();
	private:
		void printAvaliableExtensions();

	private:
		VkInstance mInstance{ VK_NULL_HANDLE };
		bool mEnableValidationLayer{ false };
		VkDebugUtilsMessengerEXT mDebugger{ VK_NULL_HANDLE };
	};
}
