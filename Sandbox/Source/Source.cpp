#include <iostream>
#include <Core.h>
#if _DEBUG

//Allocates memory to the console
void _alocConsole() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
}
#endif

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
#if _DEBUG
	_alocConsole();
#endif

	Core * core = new Core();
	if (SUCCEEDED(core->Init(hInstance)))
	{
		Window * wnd = Window::GetInstance();
		while (core->Running())
		{
			
		}
	}

	delete core;

	return 0;

}