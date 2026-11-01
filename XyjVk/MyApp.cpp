#include "MyApp.h"
MyApp* g_app = nullptr;
//*---------------------------------------------------------------------*/
void MyApp::Init(HINSTANCE hInstance,int width, int height)
{
	mhInstance = hInstance;
    mWidth = width;
    mHeight = height;
#ifdef NDEBUG
    const bool m_EnableValidationLayers = false;
#else
    const bool m_EnableValidationLayers = true;
#endif

	InitWindow();
	InitVulkan();
	mInitialized.store(true);
}
//*---------------------------------------------------------------------*/
LRESULT CALLBACK MyApp::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_SIZE:
        if (g_app && g_app->getIsInitialized())
            g_app->setFramebufferIsResized(true);
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
//*---------------------------------------------------------------------*/
void MyApp::InitWindow()
{
	const LPCWSTR className = L"MyAppClassName";

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = mhInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wc.lpszClassName = className;
    if (!RegisterClassExW(&wc))
        throw std::runtime_error("RegisterClassExW failed");

    RECT rect{ 0, 0, mWidth, mHeight };

    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    mhWnd = CreateWindowExW(
        0, className, L"我的 Vulkan 应用",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, mhInstance, nullptr);
    if (!mhWnd)
        throw std::runtime_error("CreateWindowExW failed");

    ShowWindow(mhWnd, SW_SHOWNORMAL);
    UpdateWindow(mhWnd);
}
//*---------------------------------------------------------------------*/
void MyApp::InitVulkan()
{
    mInstance = Engine::Instance::create(true);
    mSurface = Engine::WindowSurface::create(mInstance, mhWnd, mhInstance);
    mDevice = Engine::Device::create(mInstance, mSurface);
    mCommandPool = Engine::CommandPool::create(mDevice);
    mSwapChain = Engine::SwapChain::create(mDevice, mSurface, mWidth, mHeight);
    mRenderPass = Engine::RenderPass::create(mDevice);
}
//*---------------------------------------------------------------------*/
void MyApp::MainLoop() {
    MSG msg;
    while (true) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        //if (IsIconic(m_hWnd))) {
        //    Sleep(16);
        //    continue;
        //}
        //if (framebufferResized_) recreateSwapchain();
        ///drawFrame();
    }
}
//*---------------------------------------------------------------------*/
void MyApp::Cleanup()
{

}
//*---------------------------------------------------------------------*/