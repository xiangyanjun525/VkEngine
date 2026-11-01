#pragma once
#include "EngineInclude.h"
class MyApp
{
public:
	void Init(HINSTANCE hInstance, int width, int height);
	void MainLoop();
	void Cleanup();

	[[nodiscard]] bool getIsInitialized() const { return mInitialized; }
	[[nodiscard]] bool getIsFramebufferIsResized() const { return mFramebufferResized; }
	[[nodiscard]] bool getIsEnableLayer() const { return mEnableValidationLayers; }

	void setFramebufferIsResized(bool resized) { mFramebufferResized.store(resized); }
private:
	void InitWindow();
	void InitVulkan();

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	HINSTANCE mhInstance = nullptr;
	HWND      mhWnd = nullptr;

	Engine::Instance::Ptr mInstance{nullptr};
	Engine::WindowSurface::Ptr mSurface{ nullptr };
	Engine::Device::Ptr mDevice{nullptr};
	Engine::CommandPool::Ptr mCommandPool{ nullptr };
	Engine::SwapChain::Ptr mSwapChain{ nullptr };
	Engine::RenderPass::Ptr mRenderPass{ nullptr };

	std::atomic<bool> mInitialized = false;
	std::atomic<bool> mFramebufferResized = false;

	int    mWidth = 0;
	int    mHeight = 0;
	bool   mEnableValidationLayers = false;
};

extern MyApp* g_app;

