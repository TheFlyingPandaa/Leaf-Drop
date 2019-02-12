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

void printFrameTime(double dt);

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
		Camera camArr[10];

		for (int i = 0; i < 10; i++)
		{
			camArr[i].CreateProjectionMatrix(0.01f, 1000.0f);
			float x = (FLOAT)(rand() % 98 + 2) * (rand() % 2 ? 1 : -1);
			float y = (FLOAT)(rand() % 98 + 2) * (rand() % 2 ? 1 : -1);
			float z = (FLOAT)(rand() % 98 + 2) * (rand() % 2 ? 1 : -1);

			camArr[i].SetPosition(x, y, z);

			x = (FLOAT)(rand() % 500) * (rand() % 2 ? 1 : -1);
			y = (FLOAT)(rand() % 500) * (rand() % 2 ? 1 : -1);
			z = (FLOAT)(rand() % 500) * (rand() % 2 ? 1 : -1);

			camArr[i].SetDirection(x, y, z);

		}

		cam.CreateProjectionMatrix(0.01f, 1000.0f);
		cam.SetPosition(0, 0, 0);
		cam.SetAsActiveCamera();

		StaticMesh * m = new StaticMesh();
		Texture * t = new Texture[3];
		t[0].LoadTexture(L"..\\Assets\\Textures\\Brick\\Brick_diffuse.bmp");
		t[1].LoadTexture(L"..\\Assets\\Textures\\Brick\\Brick_normal.bmp");
		t[2].LoadTexture(L"..\\Assets\\Textures\\Brick\\Brick_metallic.bmp");

		m->LoadMesh("..\\Assets\\Models\\Cube.fbx");
		
		const UINT NUMBER_OF_DRAWABLES = 100;

		Drawable * d = new Drawable[NUMBER_OF_DRAWABLES];
		for (int i = 0; i < NUMBER_OF_DRAWABLES; i++)
		{
			d[i].SetTexture(&t[0]);
			d[i].SetNormal(&t[1]);
			d[i].SetMetallic(&t[2]);
			d[i].SetMesh(m);

			float x = (FLOAT)(rand() % 98 + 2) * (rand() % 2 ? 1 : -1);
			float y = (FLOAT)(rand() % 98 + 2) * (rand() % 2 ? 1 : -1);
			float z = (FLOAT)(rand() % 98 + 2) * (rand() % 2 ? 1 : -1);

			float scl = (FLOAT)(rand() % 3 + 1);

			d[i].SetPosition(x, y, z);
			d[i].SetScale(scl, scl, scl);
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
			static const float SPRINT_SPEED = 20.0f;
			static const float MOUSE_SENSITIVITY = 0.05f;

			const double dt = timer.Stop();

			printFrameTime(dt);

			DirectX::XMFLOAT3 moveDir(0.0f, 0.0f, 0.0f);
			DirectX::XMFLOAT3 rotDir(0.0f, 0.0f, 0.0f);

			if (Camera::GetActiveCamera() == &cam)
			{
				if (wnd->IsKeyPressed(Input::W))
					moveDir.z += (MOVE_SPEED + SPRINT_SPEED * wnd->IsKeyPressed(Input::SHIFT));
				if (wnd->IsKeyPressed(Input::A))
					moveDir.x -= (MOVE_SPEED + SPRINT_SPEED * wnd->IsKeyPressed(Input::SHIFT));
				if (wnd->IsKeyPressed(Input::S))
					moveDir.z -= (MOVE_SPEED + SPRINT_SPEED * wnd->IsKeyPressed(Input::SHIFT));
				if (wnd->IsKeyPressed(Input::D))
					moveDir.x += (MOVE_SPEED + SPRINT_SPEED * wnd->IsKeyPressed(Input::SHIFT));

				if (wnd->IsKeyPressed(Input::SPACE))
					moveDir.y += (MOVE_SPEED + SPRINT_SPEED * wnd->IsKeyPressed(Input::SHIFT));
				if (wnd->IsKeyPressed(Input::CTRL))
					moveDir.y -= (MOVE_SPEED + SPRINT_SPEED * wnd->IsKeyPressed(Input::SHIFT));

				float deltaMouseX = mouseThisFrame.x - mousePosLastFrame.x;
				float deltaMouseY = mouseThisFrame.y - mousePosLastFrame.y;

				rotDir.y = DirectX::XMConvertToRadians(deltaMouseX) * MOUSE_SENSITIVITY;
				rotDir.x = DirectX::XMConvertToRadians(deltaMouseY) * MOUSE_SENSITIVITY;
				static bool moveCamera = true;

				if (moveCamera)
				{
					cam.Rotate(rotDir);

					wnd->ResetMouse();
				}
				if (wnd->IsKeyPressed(Input::Z))
					moveCamera = true;
				if (wnd->IsKeyPressed(Input::X))
					moveCamera = false;

				moveDir.x *= (FLOAT)dt;
				moveDir.y *= (FLOAT)dt;
				moveDir.z *= (FLOAT)dt;

				cam.Translate(moveDir, false);


			}

			if (wnd->IsKeyPressed(Input::ESCAPE))
				wnd->Close();

			if (wnd->IsKeyPressed(Input::KEY_0))
				camArr[0].SetAsActiveCamera();
			else if (wnd->IsKeyPressed(Input::KEY_1))
				camArr[1].SetAsActiveCamera();
			else if (wnd->IsKeyPressed(Input::KEY_2))
				camArr[2].SetAsActiveCamera();
			else if (wnd->IsKeyPressed(Input::KEY_3))
				camArr[3].SetAsActiveCamera();
			else if (wnd->IsKeyPressed(Input::KEY_4))
				camArr[4].SetAsActiveCamera();
			else if (wnd->IsKeyPressed(Input::KEY_5))
				camArr[5].SetAsActiveCamera();
			else if (wnd->IsKeyPressed(Input::KEY_6))
				camArr[6].SetAsActiveCamera();
			else if (wnd->IsKeyPressed(Input::KEY_7))
				camArr[7].SetAsActiveCamera();
			else if (wnd->IsKeyPressed(Input::KEY_8))
				camArr[8].SetAsActiveCamera();
			else if (wnd->IsKeyPressed(Input::KEY_9))
				camArr[9].SetAsActiveCamera();
			else if (wnd->IsKeyPressed(Input::BACKSPACE))
				cam.SetAsActiveCamera();



		
			rot += 1.0f * (FLOAT)dt;

			for (int i = 0; i < NUMBER_OF_DRAWABLES; i++)
			{
				//d[i].SetRotation(0, rot, -rot);
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
		delete[] t;
		delete[] d;
	}
	core->Release();
	delete core;
	 
	return 0;
}

void printFrameTime(double dt)
{
	static int			counter = 0;
	static const int	MAX_DATA = 100;
	static double		allTime = 0.0;

	allTime += dt * 1000.0;
	counter++;

	if (counter >= MAX_DATA)
	{
		counter = 0;

		allTime /= MAX_DATA;

		Window::GetInstance()->SetWindowTitle("Frame Time: " + std::to_string(allTime) + " ms");

		allTime = 0.0;
	}
}