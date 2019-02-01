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

	srand(static_cast<UINT>(time(0)));

#if _DEBUG
	_alocConsole();
#endif
	Core * core = new Core();
	

	Timer timer;
	
	if (SUCCEEDED(core->Init(hInstance)))
	{
		Camera cam;
		cam.CreateProjectionMatrix();
		cam.SetPosition(0, 0, 0);
		cam.SetAsActiveCamera();

		StaticMesh * m = new StaticMesh();
		Texture * t = new Texture();
		t->LoadTexture(L"..\\Assets\\Textures\\BoobieWaSHere.png");
		m->LoadMesh("..\\Assets\\Models\\Cube.fbx");
		
		const UINT NUMBER_OF_DRAWABLES = 16384;


		Drawable * d = new Drawable[NUMBER_OF_DRAWABLES];
		for (int i = 0; i < NUMBER_OF_DRAWABLES; i++)
		{
			d[i].SetTexture(t);
			d[i].SetMesh(m);

			float x = (FLOAT)(rand() % 98 + 2) * (rand() % 2 ? 1 : -1);
			float y = (FLOAT)(rand() % 98 + 2) * (rand() % 2 ? 1 : -1);
			float z = (FLOAT)(rand() % 98 + 2) * (rand() % 2 ? 1 : -1);



			d[i].SetPosition(x, y, z);
		}
		
		

		float rot = 0;
		timer.Start();

		Window * wnd = Window::GetInstance();

		POINT mpStart = wnd->GetWindowSize();
		DirectX::XMFLOAT2 mousePosLastFrame = { (float)mpStart.x, (float)mpStart.y };
		mousePosLastFrame.x /= 2;
		mousePosLastFrame.y /= 2;

		wnd->ResetMouse();

		while (core->Running())
		{
			POINT mp = wnd->GetMousePosition();
			DirectX::XMFLOAT2 mouseThisFrame = { (float)mp.x, (float)mp.y };
			static const float MOVE_SPEED = 10.0f;
			static const float MOUSE_SENSITIVITY = 0.05f;

			
			const double dt = timer.Stop();

			DirectX::XMFLOAT3 moveDir(0.0f, 0.0f, 0.0f);
			DirectX::XMFLOAT3 rotDir(0.0f, 0.0f, 0.0f);

			if (wnd->IsKeyPressed('W'))
				moveDir.z += MOVE_SPEED;
			if (wnd->IsKeyPressed('A'))
				moveDir.x -= MOVE_SPEED;
			if (wnd->IsKeyPressed('S'))
				moveDir.z -= MOVE_SPEED;
			if (wnd->IsKeyPressed('D'))
				moveDir.x += MOVE_SPEED;
			
			if (wnd->IsKeyPressed('E'))
				moveDir.y += MOVE_SPEED;
			if (wnd->IsKeyPressed('Q'))
				moveDir.y -= MOVE_SPEED;

			float deltaMouseX = mouseThisFrame.x - mousePosLastFrame.x;
			float deltaMouseY = mouseThisFrame.y - mousePosLastFrame.y;

			rotDir.y = DirectX::XMConvertToRadians(deltaMouseX) * MOUSE_SENSITIVITY;
			rotDir.x = DirectX::XMConvertToRadians(deltaMouseY) * MOUSE_SENSITIVITY;

			cam.Rotate(rotDir);

			wnd->ResetMouse();

			moveDir.x *= (FLOAT)dt;
			moveDir.y *= (FLOAT)dt;
			moveDir.z *= (FLOAT)dt;

			cam.Translate(moveDir, false);

			rot += 1.0f * (FLOAT)dt;

			for (int i = 0; i < NUMBER_OF_DRAWABLES; i++)
			{
				d[i].SetRotation(0, rot, -rot);
				d[i].Update();
				d[i].Draw();
			}
			
			if (FAILED(core->Flush()))
			{
				DEBUG::CreateError("LOL");
				break;
			}

			
		}
		core->ClearGPU();
		delete m;
		delete t;
		delete[] d;
	}
	core->Release();
	delete core;

	return 0;

}