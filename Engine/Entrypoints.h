#define VULKAN_EXAMPLE_MAIN()																		\
VulkanExample *vulkanExample;																		\
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
#ifdef _DEBUG																							\
		AllocConsole();																						\
		FILE* fDummy = nullptr;																				\
		freopen_s(&fDummy, "CONOUT$", "w", stdout);															\
		freopen_s(&fDummy, "CONOUT$", "w", stderr);															\
		freopen_s(&fDummy, "CONIN$", "r", stdin);															\
#endif																									\
			for (int32_t i = 0; i < __argc; i++) { VulkanExample::args.push_back(__argv[i]); };  			\
				vulkanExample = new VulkanExample();															\
				vulkanExample->setupWindow(hInstance, WndProc);													\
				vulkanExample->initVulkan();																	\
				vulkanExample->prepare();																		\
				vulkanExample->renderLoop();																	\
				delete(vulkanExample);																			\
				return 0;																						\
		}
