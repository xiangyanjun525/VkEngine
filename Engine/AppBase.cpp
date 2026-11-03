#include "AppBase.h"

namespace Engine
{
	//*---------------------------------------------------------------------*/
	AppBase::~AppBase()
	{
		if (mDevice && mDevice->getDevice() != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(mDevice->getDevice());
		}

		// Destroy resources in dependency order (children before parents).
		// All objects are RAII-wrapped, so resetting the shared_ptrs frees them.
		mDrawCmdBuffers = {};
		mWaitFences = {};
		mPresentCompleteSemaphores = {};
		mRenderCompleteSemaphores.clear();
		mFramebuffers.clear();
		mDepthStencils.clear();
		mRenderPass.reset();
		mSwapChain.reset();
		mCommandPool.reset();
		mCamera.reset();
		mDevice.reset();
		mSurface.reset();
		mInstance.reset();
	}
	//*---------------------------------------------------------------------*/
	static std::wstring utf8ToWide(const std::string& utf8)
	{
		if (utf8.empty())
			return std::wstring();
		int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
		std::wstring wide(len, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wide[0], len);
		wide.resize(len - 1); // drop the implicit null terminator
		return wide;
	}
	//*---------------------------------------------------------------------*/
	HWND AppBase::setupWindow(HINSTANCE hinstance, WNDPROC wndproc)
	{
		this->mWindowInstance = hinstance;

		// mTitle / mName are UTF-8 std::strings (the source is compiled with /utf-8).
		// Convert them to wide strings for the Unicode Win32 API so non-ASCII titles
		// render correctly instead of showing mojibake.
		std::wstring className = utf8ToWide(mName);
		std::wstring windowTitle = utf8ToWide(mTitle);

		WNDCLASSEXW wndClass{};
		wndClass.cbSize = sizeof(WNDCLASSEXW);
		wndClass.style = CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = wndproc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = hinstance;
		wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = className.c_str();
		wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

		if (!RegisterClassExW(&wndClass))
		{
			std::cout << "Could not register window class!\n";
			fflush(stdout);
			exit(1);
		}

		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		if (mSettings.fullscreen)
		{
			if ((mWidth != (uint32_t)screenWidth) && (mHeight != (uint32_t)screenHeight))
			{
				DEVMODE dmScreenSettings;
				memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
				dmScreenSettings.dmSize = sizeof(dmScreenSettings);
				dmScreenSettings.dmPelsWidth = mWidth;
				dmScreenSettings.dmPelsHeight = mHeight;
				dmScreenSettings.dmBitsPerPel = 32;
				dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				{
					if (MessageBox(NULL, "Fullscreen Mode not supported!\n Switch to window mode?", "Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
					{
						mSettings.fullscreen = false;
					}
					else
					{
						return nullptr;
					}
				}
				screenWidth = mWidth;
				screenHeight = mHeight;
			}

		}

		DWORD dwExStyle;
		DWORD dwStyle;

		if (mSettings.fullscreen)
		{
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		}
		else
		{
			dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		}

		RECT windowRect{
			.left = 0L,
			.top = 0L,
			.right = mSettings.fullscreen ? (long)screenWidth : (long)mWidth,
			.bottom = mSettings.fullscreen ? (long)screenHeight : (long)mHeight
		};
		AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

		mWindow = CreateWindowExW(0,
			className.c_str(),
			windowTitle.c_str(),
			dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			0,
			0,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			NULL,
			NULL,
			hinstance,
			NULL);

		if (!mWindow)
		{
			std::cerr << "Could not create window!\n";
			fflush(stdout);
			return nullptr;
		}

		if (!mSettings.fullscreen)
		{
			// Center on screen
			uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
			uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
			SetWindowPos(mWindow, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}

		ShowWindow(mWindow, SW_SHOW);
		SetForegroundWindow(mWindow);
		SetFocus(mWindow);

		return mWindow;
	}
	//*---------------------------------------------------------------------*/
	void AppBase::handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_CLOSE:
			mPrepared = false;
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		case WM_PAINT:
			ValidateRect(mWindow, NULL);
			break;
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE) {
				PostQuitMessage(0);
			}
			break;
		case WM_LBUTTONDOWN:
			mMouseState.position = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
			mMouseState.buttons.left = true;
			break;
		case WM_RBUTTONDOWN:
			mMouseState.position = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
			mMouseState.buttons.right = true;
			break;
		case WM_MBUTTONDOWN:
			mMouseState.position = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
			mMouseState.buttons.middle = true;
			break;
		case WM_LBUTTONUP:
			mMouseState.buttons.left = false;
			break;
		case WM_RBUTTONUP:
			mMouseState.buttons.right = false;
			break;
		case WM_MBUTTONUP:
			mMouseState.buttons.middle = false;
			break;
		case WM_MOUSEWHEEL:
		{
			short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			mCamera->translate(glm::vec3(0.0f, 0.0f, (float)wheelDelta * 0.005f));
			break;
		}
		case WM_MOUSEMOVE:
		{
			handleMouseMove(LOWORD(lParam), HIWORD(lParam));
			break;
		}
		case WM_SIZE:
			if ((mPrepared) && (wParam != SIZE_MINIMIZED))
			{
				if ((m_Resizing) || ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)))
				{
					mDestWidth = LOWORD(lParam);
					mDestHeight = HIWORD(lParam);
					windowResize();
				}
			}
			break;
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO minMaxInfo = (LPMINMAXINFO)lParam;
			minMaxInfo->ptMinTrackSize.x = 64;
			minMaxInfo->ptMinTrackSize.y = 64;
			break;
		}
		case WM_ENTERSIZEMOVE:
			m_Resizing = true;
			break;
		case WM_EXITSIZEMOVE:
			m_Resizing = false;
			break;
		}
	}
	//*---------------------------------------------------------------------*/
	void AppBase::handleMouseMove(int32_t x, int32_t y)
	{
		int32_t dx = (int32_t)mMouseState.position.x - x;
		int32_t dy = (int32_t)mMouseState.position.y - y;

		if (mMouseState.buttons.left) {
			mCamera->rotate(glm::vec3(dy * mCamera->mRotationSpeed, -dx * mCamera->mRotationSpeed, 0.0f));
		}
		if (mMouseState.buttons.right) {
			mCamera->translate(glm::vec3(-0.0f, 0.0f, dy * 0.005f));
		}
		if (mMouseState.buttons.middle) {
			mCamera->translate(glm::vec3(-dx * 0.005f, -dy * 0.005f, 0.0f));
		}
		mMouseState.position = glm::vec2((float)x, (float)y);
	}
	//*---------------------------------------------------------------------*/
	bool AppBase::initVulkan()
	{
#ifdef NDEBUG
		m_EnableValidationLayers = false;
#else
		m_EnableValidationLayers = true;
#endif

		mInstance = Instance::create(m_EnableValidationLayers);

		mSurface = WindowSurface::create(mInstance, mWindow, mWindowInstance);

		mDevice = Device::create(mInstance, mSurface);

		mCommandPool = CommandPool::create(mDevice);

		if (!mCamera) {
			mCamera = Camera::create();
		}

		return true;
	}
	//*---------------------------------------------------------------------*/
	void AppBase::createSwapChain()
	{
		mSwapChain = SwapChain::create(mDevice, mSurface, mWidth, mHeight, mSettings.vsync, mSettings.fullscreen);
	}
	//*---------------------------------------------------------------------*/
	void AppBase::createCommandBuffers()
	{
		for (uint32_t i = 0; i < maxConcurrentFrames; i++) {
			mDrawCmdBuffers[i] = CommandBuffer::create(mDevice, mCommandPool);
		}
	}
	//*---------------------------------------------------------------------*/
	void AppBase::createSynchronizationPrimitives()
	{
		// Wait fences to sync command buffer access
		for (uint32_t i = 0; i < maxConcurrentFrames; i++) {
			mWaitFences[i] = Fence::create(mDevice, true);
		}
		// Used to ensure that image presentation is complete before starting to submit again
		for (uint32_t i = 0; i < maxConcurrentFrames; i++) {
			mPresentCompleteSemaphores[i] = Semaphore::create(mDevice);
		}
		// Semaphore used to ensure that all commands submitted have been finished before submitting the image to the queue
		mRenderCompleteSemaphores.resize(mSwapChain->getImageCount());
		for (uint32_t i = 0; i < mSwapChain->getImageCount(); i++) {
			mRenderCompleteSemaphores[i] = Semaphore::create(mDevice);
		}
	}
	//*---------------------------------------------------------------------*/
	void AppBase::setupDepthStencil()
	{
		// One depth image per swapchain image. With maxConcurrentFrames frames in
		// flight, two command buffers render concurrently into different framebuffers,
		// so they must not share a single depth attachment.
		mDepthStencils.resize(mSwapChain->getImageCount());
		for (uint32_t i = 0; i < mSwapChain->getImageCount(); i++) {
			mDepthStencils[i] = Utils::createDepthImage(mDevice, mWidth, mHeight, VK_SAMPLE_COUNT_1_BIT);
		}
	}
	//*---------------------------------------------------------------------*/
	void AppBase::setupRenderPass()
	{
		// Color attachment
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = mSwapChain->getFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Depth attachment
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = mDepthStencils[0]->getFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference{};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference{};
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		SubPass subPass{};
		subPass.addColorAttachmentReference(colorReference);
		subPass.setDepthStencilAttachmentReference(depthReference);
		subPass.buildSubPassDescription();

		// Subpass dependencies for layout transitions
		VkSubpassDependency depthDependency{};
		depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		depthDependency.dstSubpass = 0;
		depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depthDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

		VkSubpassDependency colorDependency{};
		colorDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		colorDependency.dstSubpass = 0;
		colorDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		colorDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		colorDependency.srcAccessMask = 0;
		colorDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

		mRenderPass = RenderPass::create(mDevice);
		mRenderPass->addAttachment(colorAttachment);
		mRenderPass->addAttachment(depthAttachment);
		mRenderPass->AddSubPass(subPass);
		mRenderPass->addDependency(depthDependency);
		mRenderPass->addDependency(colorDependency);
		mRenderPass->buildRenderPass();
	}
	//*---------------------------------------------------------------------*/
	void AppBase::setupFrameBuffer()
	{
		const auto& imageViews = mSwapChain->getImageViews();
		const VkExtent2D extent = mSwapChain->getExtent();

		mFramebuffers.resize(imageViews.size());
		for (uint32_t i = 0; i < imageViews.size(); i++) {
			std::vector<VkImageView> attachments{ imageViews[i], mDepthStencils[i]->getImageView() };
			mFramebuffers[i] = Framebuffer::create(mDevice, attachments, mRenderPass, extent.width, extent.height);
		}
	}
	//*---------------------------------------------------------------------*/
	void AppBase::prepare()
	{
		createSwapChain();
		createCommandBuffers();
		createSynchronizationPrimitives();
		setupDepthStencil();
		setupRenderPass();
		setupFrameBuffer();

		// Note: derived classes that override prepare() must call AppBase::prepare()
		// and set mPrepared = true once all of their own resources are ready.
	}
	//*---------------------------------------------------------------------*/
	void AppBase::windowResize()
	{
		if (!mPrepared) {
			return;
		}
		mPrepared = false;

		// Ensure all operations on the device have been finished before destroying resources
		vkDeviceWaitIdle(mDevice->getDevice());

		// Destroy framebuffers before the image views they reference (swapchain images
		// and the depth attachment), otherwise the image views would be destroyed while
		// still in use.
		mFramebuffers.clear();
		mDepthStencils.clear();
		mSwapChain.reset();

		// Recreate the swap chain
		mWidth = mDestWidth;
		mHeight = mDestHeight;
		createSwapChain();
		setupDepthStencil();
		setupFrameBuffer();

		// Recreate the synchronization primitives (their count depends on the swap chain)
		mWaitFences = {};
		mPresentCompleteSemaphores = {};
		mRenderCompleteSemaphores.clear();
		createSynchronizationPrimitives();

		vkDeviceWaitIdle(mDevice->getDevice());

		if ((mWidth > 0) && (mHeight > 0)) {
			mCamera->updateAspectRatio((float)mWidth / (float)mHeight);
		}

		mPrepared = true;
	}
	//*---------------------------------------------------------------------*/
	void AppBase::nextFrame()
	{
		render();
	}
	//*---------------------------------------------------------------------*/
	void AppBase::renderLoop()
	{
		mDestWidth = mWidth;
		mDestHeight = mHeight;

		MSG msg;
		bool quitMessageReceived = false;
		while (!quitMessageReceived) {
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_QUIT) {
					quitMessageReceived = true;
					break;
				}
			}
			if (mPrepared && !IsIconic(mWindow)) {
				nextFrame();
			}
		}

		// Flush device to make sure all resources can be freed
		if (mDevice && mDevice->getDevice() != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(mDevice->getDevice());
		}
	}
	//*---------------------------------------------------------------------*/
	void AppBase::prepareFrame(bool waitForFence/* = true*/)
	{
		// Ensure command buffer execution has finished
		if (waitForFence) {
			mWaitFences[mCurrentBuffer]->block();
			mWaitFences[mCurrentBuffer]->resetFence();
		}

		// Acquire the next image from the swap chain
		VkResult result = mSwapChain->acquireNextImage(
			mPresentCompleteSemaphores[mCurrentBuffer]->getSemaphore(),
			mCurrentImageIndex);

		// Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE)
		// If no longer optimal (VK_SUBOPTIMAL_KHR), wait until submitFrame() in case number of swapchain images will change on resize
		if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				windowResize();
			}
			return;
		}
		else {
			VK_CHECK_RESULT(result);
		}
	}
	//*---------------------------------------------------------------------*/
	void AppBase::submitFrame(bool skipQueueSubmit/* = false*/)
	{
		VkSemaphore presentCompleteSemaphore = mPresentCompleteSemaphores[mCurrentBuffer]->getSemaphore();
		VkSemaphore renderCompleteSemaphore = mRenderCompleteSemaphores[mCurrentImageIndex]->getSemaphore();

		if (!skipQueueSubmit) {
			const VkPipelineStageFlags waitPipelineStage{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			VkCommandBuffer drawCmdBuffer = mDrawCmdBuffers[mCurrentBuffer]->getCommandBuffer();

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
			submitInfo.pWaitDstStageMask = &waitPipelineStage;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &drawCmdBuffer;
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &renderCompleteSemaphore;
			VK_CHECK_RESULT(vkQueueSubmit(mDevice->getGraphicAndCompteQueue(), 1, &submitInfo, mWaitFences[mCurrentBuffer]->getFence()));
		}

		VkSwapchainKHR swapChain = mSwapChain->getSwapChain();
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain;
		presentInfo.pImageIndices = &mCurrentImageIndex;
		VkResult result = vkQueuePresentKHR(mDevice->getPresentQueue(), &presentInfo);

		// Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
		if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
			windowResize();
			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				return;
			}
		}
		else {
			VK_CHECK_RESULT(result);
		}
		// Select the next frame to render to, based on the max. no. of concurrent frames
		mCurrentBuffer = (mCurrentBuffer + 1) % maxConcurrentFrames;
	}
	//*---------------------------------------------------------------------*/
}
