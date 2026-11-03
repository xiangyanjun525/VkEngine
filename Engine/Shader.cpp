#include "Shader.h"

namespace Engine
{
	Shader::Shader(const Device::Ptr& device,
		std::vector<char>& codeBuffer,
		VkShaderStageFlagBits shaderStage,
		const std::string& entryPoint)
	{
		mDevice = device;
		mShaderStage = shaderStage;
		mEntryPoint = entryPoint;
		VkShaderModuleCreateInfo shaderCreateInfo = {};
		shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderCreateInfo.codeSize = codeBuffer.size();
		shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(codeBuffer.data());
		if (vkCreateShaderModule(mDevice->getDevice(), &shaderCreateInfo, nullptr, &mShaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Error:failed to create shader module");
		}
	}

	Shader::~Shader()
	{
		vkDestroyShaderModule(mDevice->getDevice(), mShaderModule, nullptr);
	}
}