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
	
	
	Timer timer;
	
	if (SUCCEEDED(core->Init(hInstance)))
	{
		StaticMesh * m = new StaticMesh();
		Texture * t = new Texture();
		t->LoadTexture(L"..\\Assets\\Textures\\BoobieWaSHere.png");
		m->LoadMesh("..\\Assets\\Models\\Cube.fbx");
		Drawable d;
		Drawable d2;

		d.SetTexture(t);
		d.SetMesh(m);
		d2.SetTexture(t);
		d2.SetMesh(m);
		d2.SetScale(0.5f, 0.5f, 0.5f);
		d2.Update();
		float rot = 0;
		timer.Start();

		Window * wnd = Window::GetInstance();

		while (core->Running())
		{
			const float dt = timer.Stop();
			wnd->SetWindowTitle(std::to_string(dt));

			rot += 1.0f * dt;
			d.SetRotation(0, rot, -rot);
			d.Update();

			d.Draw();
			d2.Draw();
			if (FAILED(core->Flush()))
			{
				DEBUG::CreateError("LOL");
				break;
			}
		}
		core->ClearGPU();
		delete m;
		delete t;
	}
	core->Release();
	delete core;

	return 0;

}