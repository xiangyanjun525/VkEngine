#pragma once
#include "Base.h"
#include "Camera.h"
#include "Device.h"
#include "CommandPool.h"
#include "WindowSurface.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "Image.h"
#include "Semaphore.h"
#include "Fence.h"
#include "CommandBuffer.h"
#include "Shader.h"
#include "Utils.h"

constexpr uint32_t maxConcurrentFrames{ 2 };
namespace Engine
{
	class AppBase
	{
	public:
		AppBase() {}
		virtual ~AppBase();
		bool mPrepared = false;
		uint32_t mWidth = 1280;
		uint32_t mHeight = 720;
		bool m_EnableValidationLayers = false;

		struct Settings {
			/** @brief Set to true if fullscreen mode has been requested via command line */
			bool fullscreen = false;
			/** @brief Set to true if v-sync will be forced for the swapchain */
			bool vsync = false;
		} mSettings;

		struct {
			struct {
				bool left = false;
				bool right = false;
				bool middle = false;
			} buttons;
			glm::vec2 position;
		} mMouseState;

		std::string mTitle = "Xiangyj vkLearn Example";
		std::string mName = "Xiangyj vkLearn Example";

		Camera::Ptr mCamera{ nullptr };
		Engine::Instance::Ptr mInstance{ nullptr };
		Engine::Device::Ptr mDevice{ nullptr };
		Engine::CommandPool::Ptr mCommandPool{ nullptr };
		Engine::WindowSurface::Ptr mSurface{ nullptr };
		HWND mWindow;
		HINSTANCE mWindowInstance;

		/** @brief Default clear color used by the default render pass */
		VkClearColorValue mDefaultClearColor{ { 0.025f, 0.025f, 0.025f, 1.0f } };

		HWND setupWindow(HINSTANCE hinstance, WNDPROC wndproc);
		void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual void render() = 0;

		/** @brief Prepares all Vulkan resources and functions required to run the sample */
		virtual void prepare();

		void windowResize();

		/** @brief Entry point for the main render loop */
		void renderLoop();

		/** Prepare the next frame for workload submission by acquiring the next swap chain image and waiting for the previous command buffer to finish */
		void prepareFrame(bool waitForFence = true);
		/** @brief Presents the current image to the swap chain */
		void submitFrame(bool skipQueueSubmit = false);

		bool initVulkan();

	protected:
		Engine::SwapChain::Ptr mSwapChain{ nullptr };
		Engine::RenderPass::Ptr mRenderPass{ nullptr };
		std::vector<Engine::Image::Ptr> mDepthStencils{};
		std::vector<Engine::Framebuffer::Ptr> mFramebuffers{};

		std::array<Engine::CommandBuffer::Ptr, maxConcurrentFrames> mDrawCmdBuffers{};
		std::array<Engine::Semaphore::Ptr, maxConcurrentFrames> mPresentCompleteSemaphores{};
		std::vector<Engine::Semaphore::Ptr> mRenderCompleteSemaphores{};
		std::array<Engine::Fence::Ptr, maxConcurrentFrames> mWaitFences{};

		uint32_t mCurrentImageIndex{ 0 };
		uint32_t mCurrentBuffer{ 0 };

		/** @brief (Virtual) Setup default depth and stencil views */
		virtual void setupDepthStencil();
		/** @brief (Virtual) Setup a default renderpass */
		virtual void setupRenderPass();
		/** @brief (Virtual) Setup default framebuffers for all requested swapchain images */
		virtual void setupFrameBuffer();

	private:
		void handleMouseMove(int32_t x, int32_t y);
		void nextFrame();
		void createSwapChain();
		void createCommandBuffers();
		void createSynchronizationPrimitives();

		bool		m_Resizing = false;
		uint32_t	mDestWidth{};
		uint32_t	mDestHeight{};
	};
}

#include "Entrypoints.h"
