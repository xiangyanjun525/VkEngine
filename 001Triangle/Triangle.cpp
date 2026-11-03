
#include <stdio.h>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>
#include "../Engine/EngineInclude.h"

class VulkanExample : public Engine::AppBase
{
public:
	struct ShaderData {
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
	};

	Engine::Buffer::Ptr mVertexBuffer{ nullptr };
	Engine::Buffer::Ptr mIndexBuffer{ nullptr };
	std::vector<Engine::Buffer::Ptr> mUniformBuffers{};

	Engine::UniformParameter::Ptr mUniformParam{ nullptr };
	Engine::DescriptorSetLayout::Ptr mDescriptorSetLayout{ nullptr };
	Engine::DescriptorPool::Ptr mDescriptorPool{ nullptr };
	Engine::DescriptorSet::Ptr mDescriptorSet{ nullptr };

	Engine::Pipeline::Ptr mPipeline{ nullptr };

	std::string mShaderDir = "shaders/";

	VulkanExample() : Engine::AppBase()
	{
		mTitle = "Basic indexed triangle";
		mName = "triangle";
		mDefaultClearColor = { { 0.0f, 0.0f, 0.2f, 1.0f } };

		mCamera = Engine::Camera::create();
		mCamera->mType = Engine::Camera::CameraType::lookat;
		mCamera->setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
		mCamera->setRotation(glm::vec3(0.0f));
		mCamera->setPerspective(60.0f, (float)mWidth / (float)mHeight, 1.0f, 256.0f);
	}

	void createVertexBuffer()
	{
		std::vector<Engine::Vertex_PosColor> vertices{
			{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
		};
		uint32_t vertexBufferSize = static_cast<uint32_t>(vertices.size()) * sizeof(Engine::Vertex_PosColor);
		mVertexBuffer = Engine::Utils::createVertexBuffer(mDevice, VK_SHARING_MODE_EXCLUSIVE, vertexBufferSize, vertices.data());

		std::vector<uint32_t> indices{ 0, 1, 2 };
		uint32_t indexBufferSize = static_cast<uint32_t>(indices.size()) * sizeof(uint32_t);
		mIndexBuffer = Engine::Utils::createIndexBuffer(mDevice, VK_SHARING_MODE_EXCLUSIVE, indexBufferSize, indices.data());
	}

	// One host-visible uniform buffer per frame, updated every frame.
	void createUniformBuffers()
	{
		mUniformBuffers.resize(maxConcurrentFrames);
		for (uint32_t i = 0; i < maxConcurrentFrames; i++) {
			mUniformBuffers[i] = Engine::Utils::createUniformBuffer(mDevice, VK_SHARING_MODE_EXCLUSIVE, sizeof(ShaderData));
		}
	}

	// Binding 0: uniform buffer in the vertex shader.
	void createDescriptorSetLayout()
	{
		mUniformParam = Engine::UniformParameter::create();
		mUniformParam->mBinding = 0;
		mUniformParam->mCount = 1;
		mUniformParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		mUniformParam->mStage = VK_SHADER_STAGE_VERTEX_BIT;
		mUniformParam->mSize = sizeof(ShaderData);
		mUniformParam->mBuffers = mUniformBuffers;

		mDescriptorSetLayout = Engine::DescriptorSetLayout::create(mDevice);
		mDescriptorSetLayout->build({ mUniformParam });
	}

	void createDescriptorPool()
	{
		mDescriptorPool = Engine::DescriptorPool::create(mDevice);
		mDescriptorPool->build({ mUniformParam }, static_cast<int>(maxConcurrentFrames));
	}

	// Allocate one descriptor set per frame and point it at that frame's uniform buffer.
	void createDescriptorSet()
	{
		mDescriptorSet = Engine::DescriptorSet::create(
			mDevice, { mUniformParam }, mDescriptorSetLayout, mDescriptorPool,
			static_cast<int>(maxConcurrentFrames));
	}

	void createPipeline()
	{
		mPipeline = Engine::Pipeline::create(mDevice, mRenderPass);

		// Shaders (compiled to SPIR-V offline).
		auto vertCode = Engine::Utils::readBinary(mShaderDir + "triangle.vert.spv");
		auto fragCode = Engine::Utils::readBinary(mShaderDir + "triangle.frag.spv");
		auto vertShader = Engine::Shader::create(mDevice, vertCode, VK_SHADER_STAGE_VERTEX_BIT, "main");
		auto fragShader = Engine::Shader::create(mDevice, fragCode, VK_SHADER_STAGE_FRAGMENT_BIT, "main");
		mPipeline->setShaderGroup({ vertShader, fragShader });

		// Vertex input matches triangle.vert: location 0 = position, location 1 = color.
		auto bindingDes = Engine::Vertex_PosColor::getVertexInputBindingDescriptions(VK_VERTEX_INPUT_RATE_VERTEX);
		auto attributeDes = Engine::Vertex_PosColor::getVertexInputAttributeDescriptions();
		mPipeline->mVertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDes.size());
		mPipeline->mVertexInputState.pVertexBindingDescriptions = bindingDes.data();
		mPipeline->mVertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDes.size());
		mPipeline->mVertexInputState.pVertexAttributeDescriptions = attributeDes.data();

		// Input assembly.
		mPipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		// Viewport / scissor are set dynamically per frame (see render()), so the
		// pipeline stays correct after a window resize.
		mPipeline->setDynamicStates({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });

		// Rasterization.
		mPipeline->mRasterState.polygonMode = VK_POLYGON_MODE_FILL;
		mPipeline->mRasterState.cullMode = VK_CULL_MODE_NONE;
		mPipeline->mRasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		mPipeline->mRasterState.lineWidth = 1.0f;

		// Multisampling.
		mPipeline->mSampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// Blend: no blending, write all color channels.
		VkPipelineColorBlendAttachmentState blendAttachment{};
		blendAttachment.colorWriteMask = 0xf;
		blendAttachment.blendEnable = VK_FALSE;
		mPipeline->pushBlendAttachment(blendAttachment);

		// Depth test against the default depth attachment.
		mPipeline->mDepthStencilState.depthTestEnable = VK_TRUE;
		mPipeline->mDepthStencilState.depthWriteEnable = VK_TRUE;
		mPipeline->mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		mPipeline->mDepthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
		mPipeline->mDepthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
		mPipeline->mDepthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
		mPipeline->mDepthStencilState.front = mPipeline->mDepthStencilState.back;

		// Pipeline layout uses the descriptor set layout created above.
		VkDescriptorSetLayout layout = mDescriptorSetLayout->getLayout();
		mPipeline->mLayoutState.setLayoutCount = 1;
		mPipeline->mLayoutState.pSetLayouts = &layout;

		mPipeline->build();
	}

	void prepare() override
	{
		Engine::AppBase::prepare();

		createVertexBuffer();
		createUniformBuffers();
		createDescriptorSetLayout();
		createDescriptorPool();
		createDescriptorSet();
		createPipeline();

		mPrepared = true;
	}

	void render() override
	{
		if (!mPrepared)
			return;

		prepareFrame();

		// Update the current frame's uniform buffer with the camera matrices.
		ShaderData shaderData{};
		shaderData.projectionMatrix = mCamera->mMatrices.perspective;
		shaderData.viewMatrix = mCamera->mMatrices.view;
		shaderData.modelMatrix = glm::mat4(1.0f);
		mUniformBuffers[mCurrentBuffer]->updateBufferByMap(&shaderData, sizeof(ShaderData));

		// Record the draw commands for the current frame.
		auto cmd = mDrawCmdBuffers[mCurrentBuffer];
		cmd->reset();
		cmd->begin();

		VkClearValue clearValues[2]{};
		clearValues[0].color = mDefaultClearColor;
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = mRenderPass->getRenderPass();
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = mSwapChain->getExtent();
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = mFramebuffers[mCurrentImageIndex]->getFramebuffer();

		cmd->beginRenderPass(renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)mWidth;
		viewport.height = (float)mHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cmd->setViewport(viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { mWidth, mHeight };
		cmd->setScissor(scissor);

		cmd->bindPipeline(mPipeline->getPipeline(), VK_PIPELINE_BIND_POINT_GRAPHICS);
		cmd->bindVertexBuffer({ mVertexBuffer->getBuffer() });
		cmd->bindIndexBuffer(mIndexBuffer->getBuffer(), VK_INDEX_TYPE_UINT32);
		cmd->bindDescriptorSet(mPipeline->getLayout(), mDescriptorSet->getDescriptorSet(mCurrentBuffer), VK_PIPELINE_BIND_POINT_GRAPHICS);
		cmd->drawIndex(3);
		cmd->endRenderPass();
		cmd->end();

		submitFrame();
	}
};

VulkanExample* vulkanExample;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (vulkanExample != NULL)
	{
		vulkanExample->handleMessages(hWnd, uMsg, wParam, lParam);
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR, _In_ int)
{
#ifdef _DEBUG
	AllocConsole();
	FILE* fDummy = nullptr;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONIN$", "r", stdin);
#endif

	vulkanExample = new VulkanExample();
	vulkanExample->setupWindow(hInstance, WndProc);
	vulkanExample->initVulkan();
	vulkanExample->prepare();
	vulkanExample->renderLoop();
	delete(vulkanExample);
	return 0;
}
