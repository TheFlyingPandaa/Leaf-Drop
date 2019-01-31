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
		Camera cam;
		cam.CreateProjectionMatrix();
		cam.SetPosition(0, 0, 0);
		cam.SetAsActiveCamera();

		StaticMesh * m = new StaticMesh();
		Texture * t = new Texture();
		t->LoadTexture(L"..\\Assets\\Textures\\BoobieWaSHere.png");
		m->LoadMesh("..\\Assets\\Models\\Cube.fbx");
		
		Drawable d[4];
		for (int i = 0; i < 4; i++)
		{
			d[i].SetTexture(t);
			d[i].SetMesh(m);
		}
		
		float dist = 5.0;
		DirectX::XMFLOAT4 pos(dist, 0.0f, 0.0f, 1.0f);
		d[0].SetPosition(pos);

		pos.x = -dist;
		d[1].SetPosition(pos);

		pos.x = 0;
		pos.z = -dist;
		d[2].SetPosition(pos);

		pos.z = dist;
		d[3].SetPosition(pos);


		float rot = 0;
		timer.Start();

		Window * wnd = Window::GetInstance();

		UINT2 mpStart = wnd->GetWindowSize();
		DirectX::XMFLOAT2 mousePosLastFrame = { (float)mpStart.x, (float)mpStart.y };
		mousePosLastFrame.x /= 2;
		mousePosLastFrame.y /= 2;

		wnd->ResetMouse();

		while (core->Running())
		{
			UINT2 mp = wnd->GetMousePosition();
			DirectX::XMFLOAT2 mouseThisFrame = { (float)mp.x, (float)mp.y };


			const double dt = timer.Stop();
			wnd->SetWindowTitle(std::to_string(dt));

			DirectX::XMFLOAT3 moveDir(0.0f, 0.0f, 0.0f);
			DirectX::XMFLOAT3 rotDir(0.0f, 0.0f, 0.0f);

			if (wnd->IsKeyPressed('W'))
				moveDir.z += 1.0f;
			if (wnd->IsKeyPressed('A'))
				moveDir.x -= 1.0f;
			if (wnd->IsKeyPressed('S'))
				moveDir.z -= 1.0f;
			if (wnd->IsKeyPressed('D'))
				moveDir.x += 1.0f;
			
			if (wnd->IsKeyPressed('E'))
				moveDir.y += 1.0f;
			if (wnd->IsKeyPressed('Q'))
				moveDir.y -= 1.0f;

			float deltaMouseX = mouseThisFrame.x - mousePosLastFrame.x;
			float deltaMouseY = mouseThisFrame.y - mousePosLastFrame.y;

			if (deltaMouseX != 0 || deltaMouseY != 0)
			{
				rotDir.y = deltaMouseX * dt;
				rotDir.x = deltaMouseY * dt;

				cam.Rotate(rotDir);
			}

			moveDir.x *= dt;
			moveDir.y *= dt;
			moveDir.z *= dt;

			cam.Translate(moveDir, false);
			



			rot += 1.0f * dt;

			for (int i = 0; i < 4; i++)
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

			wnd->ResetMouse();
		}
		core->ClearGPU();
		delete m;
		delete t;
	}
	core->Release();
	delete core;

	return 0;

}