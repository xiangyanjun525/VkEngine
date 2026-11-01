#include "AppBase.h"
#include "keycodes.hpp"

namespace Engine
{
	//*---------------------------------------------------------------------*/
	void AppBase::keyPressed(uint32_t) {}
	//*---------------------------------------------------------------------*/
	void AppBase::mouseMoved(double x, double y, bool& handled) {}
	//*---------------------------------------------------------------------*/
	HWND AppBase::setupWindow(HINSTANCE hinstance, WNDPROC wndproc)
	{
		this->mWindowInstance = hinstance;

		WNDCLASSEX wndClass{
			.cbSize = sizeof(WNDCLASSEX),
			.style = CS_HREDRAW | CS_VREDRAW,
			.lpfnWndProc = wndproc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = hinstance,
			.hIcon = LoadIcon(NULL, IDI_APPLICATION),
			.hCursor = LoadCursor(NULL, IDC_ARROW),
			.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
			.lpszMenuName = NULL,
			.lpszClassName = mName.c_str(),
			.hIconSm = LoadIcon(NULL, IDI_WINLOGO),
		};

		if (!RegisterClassEx(&wndClass))
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

		std::string windowTitle = mTitle;
		mWindow = CreateWindowEx(0,
			mName.c_str(),
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
			switch (wParam)
			{
			case KEY_P:
				mPaused = !mPaused;
				break;
			case KEY_F2:
				if (mCamera->mType == Camera::CameraType::lookat) {
					mCamera->mType = Camera::CameraType::firstperson;
				}
				else {
					mCamera->mType = Camera::CameraType::lookat;
				}
				break;
			case KEY_ESCAPE:
				PostQuitMessage(0);
				break;
			}

			if (mCamera->mType == Camera::firstperson)
			{
				switch (wParam)
				{
				case KEY_W:
					mCamera->mKeys.up = true;
					break;
				case KEY_S:
					mCamera->mKeys.down = true;
					break;
				case KEY_A:
					mCamera->mKeys.left = true;
					break;
				case KEY_D:
					mCamera->mKeys.right = true;
					break;
				}
			}

			keyPressed((uint32_t)wParam);
			break;
		case WM_KEYUP:
			if (mCamera->mType == Camera::firstperson)
			{
				switch (wParam)
				{
				case KEY_W:
					mCamera->mKeys.up = false;
					break;
				case KEY_S:
					mCamera->mKeys.down = false;
					break;
				case KEY_A:
					mCamera->mKeys.left = false;
					break;
				case KEY_D:
					mCamera->mKeys.right = false;
					break;
				}
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

		OnHandleMessage(hWnd, uMsg, wParam, lParam);
	}
	//*---------------------------------------------------------------------*/
	void AppBase::windowResized() {}
	//*---------------------------------------------------------------------*/
	void AppBase::prepare()
	{
		mCamera = Camera::create();
	}
	//*---------------------------------------------------------------------*/
	void AppBase::windowResize()
	{
		if (!mPrepared) {
			return;
		}
		mPrepared = false;
		mResized = true;

		// Ensure all operations on the device have been finished before destroying resources
		//vkDeviceWaitIdle(mDevice->getDevice);

		// Recreate swap chain
		mWidth = mDestWidth;
		mHeight = mDestHeight;

		//to do ...
	}
	//*---------------------------------------------------------------------*/
	void AppBase::renderLoop()
	{

	}
	//*---------------------------------------------------------------------*/
	void AppBase::prepareFrame(bool waitForFence/* = true*/)
	{

	}
	//*---------------------------------------------------------------------*/
	void AppBase::submitFrame(bool skipQueueSubmit/* = false*/)
	{

	}
	//*---------------------------------------------------------------------*/
	void AppBase::OnHandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {};
	//*---------------------------------------------------------------------*/
	void AppBase::handleMouseMove(int32_t x, int32_t y)
	{
		int32_t dx = (int32_t)mMouseState.position.x - x;
		int32_t dy = (int32_t)mMouseState.position.y - y;

		bool handled = false;
		mouseMoved((float)x, (float)y, handled);

		if (mMouseState.buttons.left) {
			mCamera->rotate(glm::vec3(dy * mCamera->mRotationSpeed, -dx * mCamera->mRotationSpeed, 0.0f));
		}
		if (mMouseState.buttons.right) {
			mCamera->translate(glm::vec3(-0.0f, 0.0f, dy * .005f));
		}
		if (mMouseState.buttons.middle) {
			mCamera->translate(glm::vec3(-dx * 0.005f, -dy * 0.005f, 0.0f));
		}
		mMouseState.position = glm::vec2((float)x, (float)y);
	}
	//*---------------------------------------------------------------------*/
	void AppBase::nextFrame()
	{

	}
	//*---------------------------------------------------------------------*/
}
