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
		StaticMesh * m = new StaticMesh();
		Texture * t = new Texture();
		t->LoadTexture(L"..\\Assets\\Textures\\BoobieWaSHere.png");
		m->LoadMesh("..\\Assets\\Models\\Cube.fbx");
		Drawable d;
		d.SetTexture(t);
		d.SetMesh(m);

		while (core->Running())
		{
			d.Draw();
			core->Flush();
		}
		core->ClearGPU();
		delete m;
	}
	core->Release();
	delete core;

	return 0;

}