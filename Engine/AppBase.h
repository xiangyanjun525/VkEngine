#pragma once
#include "Base.h"
#include <chrono>
#include "Camera.h"
namespace Engine
{
	class AppBase
	{
	public:
		bool mPrepared = false;
		bool mResized = false;
		uint32_t mWidth = 1280;
		uint32_t mHeight = 720;

		float mFrameTimer = 1.0f;

		struct Settings {
			/** @brief Activates validation layers (and message output) when set to true */
			bool validation = false;
			/** @brief Set to true if fullscreen mode has been requested via command line */
			bool fullscreen = false;
			/** @brief Set to true if v-sync will be forced for the swapchain */
			bool vsync = false;
			/** @brief Enable UI overlay */
			bool overlay = true;
		} mSettings;

		struct {
			struct {
				bool left = false;
				bool right = false;
				bool middle = false;
			} buttons;
			glm::vec2 position;
		} mMouseState;

		// Defines a frame rate independent timer value clamped from -1.0...1.0
		// For use in animations, rotations, etc.
		float mTimer = 0.0f;
		// Multiplier for speeding up (or slowing down) the global timer
		float mTimerSpeed = 0.25f;
		bool mPaused = false;

		std::string mTitle = "Xiangyj vkLearn Example";
		std::string mName = "Xiangyj vkLearn Example";

		Camera::Ptr mCamera{nullptr};

		HWND mWindow;
		HINSTANCE mWindowInstance;

		HWND setupWindow(HINSTANCE hinstance, WNDPROC wndproc);
		void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		virtual void render() = 0;
		/** @brief (Virtual) Called after a key was pressed, can be used to do custom key handling */
		virtual void keyPressed(uint32_t);
		/** @brief (Virtual) Called after the mouse cursor moved and before internal events (like camera rotation) is handled */
		virtual void mouseMoved(double x, double y, bool& handled);
		/** @brief (Virtual) Called when the window has been resized, can be used by the sample application to recreate resources */
		virtual void windowResized();

		/** @brief Prepares all Vulkan resources and functions required to run the sample */
		virtual void prepare();

		void windowResize();

		/** @brief Entry point for the main render loop */
		void renderLoop();

		/** Prepare the next frame for workload submission by acquiring the next swap chain image and waiting for the previous command buffer to finish */
		void prepareFrame(bool waitForFence = true);
		/** @brief Presents the current image to the swap chain */
		void submitFrame(bool skipQueueSubmit = false);

		virtual void OnHandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		void handleMouseMove(int32_t x, int32_t y);
		void nextFrame();

	protected:
		std::chrono::time_point<std::chrono::high_resolution_clock> mLastTimestamp, mPrevEnd;

		uint32_t mCurrentImageIndex{ 0 };
		uint32_t mCurrentBuffer{ 0 };
	private:
		bool		m_Resizing = false;
		uint32_t	mDestWidth{};
		uint32_t	mDestHeight{};


	};

	#define	VulkanExample vulkanExample;																		\
		LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)						\
	{																									\
		if (vulkanExample != NULL)																		\
		{																								\
			vulkanExample->handleMessages(hWnd, uMsg, wParam, lParam);									\
		}																								\
			return (DefWindowProc(hWnd, uMsg, wParam, lParam));												\
	}																									\
	int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR, _In_ int) \
	{																									\
		for (int32_t i = 0; i < __argc; i++) { VulkanExample::args.push_back(__argv[i]); };  			\
			vulkanExample = new VulkanExample();															\
			vulkanExample->initVulkan();																	\
			vulkanExample->setupWindow(hInstance, WndProc);													\
			vulkanExample->prepare();																		\
			vulkanExample->renderLoop();																	\
			delete(vulkanExample);																			\
			return 0;																						\
	}
}

