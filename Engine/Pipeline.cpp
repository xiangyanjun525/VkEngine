#include "Pipeline.h"

namespace Engine
{
	Pipeline::Pipeline(const Device::Ptr& device, const RenderPass::Ptr& renderPass)
	{
		mDevice = device;
		mRenderPass = renderPass;

		mVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mSampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		mDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		mLayoutState.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	}

	Pipeline::~Pipeline()
	{
		vkDestroyPipelineLayout(mDevice->getDevice(), mLayout, nullptr);
		vkDestroyPipeline(mDevice->getDevice(), mPipeline, nullptr);
	}

	void Pipeline::setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup)
	{
		mShaders = shaderGroup;
	}

	void Pipeline::build()
	{
		//设置Shader
		std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos{};
		for (const auto& shader : mShaders)
		{
			VkPipelineShaderStageCreateInfo shaderCreateInfo{};
			shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderCreateInfo.module = shader->getShaderModule();
			shaderCreateInfo.stage = shader->getShaderStage();
			shaderCreateInfo.pName = shader->getEntryPoint();

			shaderCreateInfos.push_back(shaderCreateInfo);
		}

		//设置视口裁剪
		mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		mViewportState.viewportCount = mViewports.empty() ? 1 : static_cast<uint32_t>(mViewports.size());
		mViewportState.pViewports = mViewports.empty() ? nullptr : mViewports.data();
		mViewportState.scissorCount = mScissors.empty() ? 1 : static_cast<uint32_t>(mScissors.size());
		mViewportState.pScissors = mScissors.empty() ? nullptr : mScissors.data();

		//动态状态（如动态视口/裁剪，可在命令缓冲里修改）
		mDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		mDynamicState.dynamicStateCount = static_cast<uint32_t>(mDynamicStates.size());
		mDynamicState.pDynamicStates = mDynamicStates.data();

		//blending 
		mBlendState.attachmentCount = static_cast<uint32_t>(mBlendAttachmentStates.size());
		mBlendState.pAttachments = mBlendAttachmentStates.data();

		//layout生成
		if (mLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(mDevice->getDevice(), mLayout, nullptr);
		}
		if (vkCreatePipelineLayout(mDevice->getDevice(), &mLayoutState, nullptr, &mLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Error: failed to create layout");
		}

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderCreateInfos.size());
		pipelineCreateInfo.pStages = shaderCreateInfos.data();

		pipelineCreateInfo.pVertexInputState = &mVertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &mAssemblyState;
		pipelineCreateInfo.pViewportState = &mViewportState;
		pipelineCreateInfo.pRasterizationState = &mRasterState;
		pipelineCreateInfo.pMultisampleState = &mSampleState;
		pipelineCreateInfo.pDepthStencilState = &mDepthStencilState;
		pipelineCreateInfo.pColorBlendState = &mBlendState;
		pipelineCreateInfo.pDynamicState = mDynamicStates.empty() ? nullptr : &mDynamicState;
		pipelineCreateInfo.layout = mLayout;
		pipelineCreateInfo.renderPass = mRenderPass->getRenderPass();
		pipelineCreateInfo.subpass = 0;

		//以存在的pipeline为基础进行创建，会更快,但是需要指定flags为 VK_PIPELINE_CREATE_DERIVATIVE_BIT
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.basePipelineIndex = -1;

		if (mPipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(mDevice->getDevice(), mPipeline, nullptr);
		}

		//pipeline cache,可以将相关数据存入缓存，在多个pipeline当中使用,也可以存到文件不同程序调用
		if (vkCreateGraphicsPipelines(mDevice->getDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Error: failed to create pipeline");
		}
	}
}