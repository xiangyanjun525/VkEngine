#pragma once
#include "Base.h"
#include "Device.h"

namespace Engine
{
	class Shader
	{
	public:
		using Ptr = std::shared_ptr<Shader>;
		static Ptr create(const Device::Ptr& device,
			std::vector<char>& codeBuffer,
			VkShaderStageFlagBits shaderStage,
			const std::string& entryPoint)
		{
			return std::make_shared<Shader>(device, codeBuffer, shaderStage, entryPoint);
		}

		Shader(const Device::Ptr& device,
			std::vector<char>& codeBuffer,
			VkShaderStageFlagBits shaderStage,
			const std::string& entryPoint);

		~Shader();


		[[nodiscard]] auto getShaderModule() const { return mShaderModule; }
		[[nodiscard]] auto getEntryPoint() const { return mEntryPoint.c_str(); }
		[[nodiscard]] auto getShaderStage() const { return mShaderStage; }
	private:
		VkShaderModule mShaderModule{ VK_NULL_HANDLE };
		Device::Ptr mDevice{ nullptr };
		std::string mEntryPoint;
		VkShaderStageFlagBits mShaderStage;
	};
}

