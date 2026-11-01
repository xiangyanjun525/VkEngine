#include "EngineInclude.h"
#include "MyApp.h"
#include <cstdio>
#include <cstdlib>
#include <exception>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine,
	int nCmdShow)
{
#ifdef _DEBUG
	AllocConsole();
	FILE* fDummy = nullptr;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONIN$", "r", stdin);
#endif

	try
	{
		g_app = new MyApp();
		g_app->Init(hInstance, 800, 600);
		g_app->MainLoop();

		g_app->Cleanup();
		delete g_app;
		g_app = nullptr;
	}
	catch (const std::exception& e)
	{
		std::cerr << "[Exception] " << e.what() << std::endl;
	}

#ifdef _DEBUG
	system("pause");
#endif

	return 0;
}