#include <iostream>
#include <Core.h>
#include "Leaf-Drop/Objects/Lights/DirectionalLight.h"
#include "Leaf-Drop/Objects/Lights/PointLight.h"
#include <algorithm>


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
	srand(0);

#if _DEBUG
	_alocConsole();
#endif
	Core * core = new Core();
	
	Timer timer;
	
	if (SUCCEEDED(core->Init(hInstance)))
	{
		Camera cam;
		
		cam.CreateProjectionMatrix(0.01f, 1000.0f);
		cam.SetPosition(0, 0, 0);
		cam.SetAsActiveCamera();

		StaticMesh * m = new StaticMesh();
		StaticMesh * m2 = new StaticMesh();
		StaticMesh * bunny = new StaticMesh();

		Texture * t = new Texture[3];
		t[0].LoadTexture(L"..\\Assets\\Textures\\Magnus_Mirror\\Mirror_diffuseOriginal.bmp");
		t[1].LoadTexture(L"..\\Assets\\Textures\\Magnus_Mirror\\Mirror_normal.bmp");
		t[2].LoadTexture(L"..\\Assets\\Textures\\Magnus_Mirror\\Mirror_metallic.bmp");

		Texture * t2 = new Texture[3];
		t2[0].LoadTexture(L"..\\Assets\\Textures\\Magnus_Brick\\Brick_diffuseOriginal.bmp");
		t2[1].LoadTexture(L"..\\Assets\\Textures\\Magnus_Brick\\Brick_normal.bmp");
		t2[2].LoadTexture(L"..\\Assets\\Textures\\Magnus_Brick\\Brick_metallic.bmp");

		Texture * bunnyTexture = new Texture[3];
		bunnyTexture[0].LoadTexture(L"..\\Assets\\Textures\\BobTheBunny\\BobTheBunny_diffuseOriginal.bmp");
		bunnyTexture[1].LoadTexture(L"..\\Assets\\Textures\\BobTheBunny\\BobTheBunny_normal.bmp");
		bunnyTexture[2].LoadTexture(L"..\\Assets\\Textures\\BobTheBunny\\BobTheBunny_metallic.bmp");

		m->LoadMesh("..\\Assets\\Models\\Cube.fbx");
		m2->LoadMesh("..\\Assets\\Models\\Sphere.fbx");
		bunny->LoadMesh("..\\Assets\\Models\\BobTheBunny.fbx");
		
		const UINT NUMBER_OF_DRAWABLES = 9;
		const UINT NUMBER_OF_LIGHTS = 500;
		const UINT MAX_DISTANCE = 50;
		const UINT MAX_LIGHT_DISTANCE = 50;

		PointLight * pointLight = new PointLight[NUMBER_OF_LIGHTS];
		for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
		{	

			float r = static_cast<float>(rand() % 100) / 100.0f;
			float g = static_cast<float>(rand() % 100) / 100.0f;
			float b = static_cast<float>(rand() % 100)  / 100.0f;

			pointLight[i].SetColor(r, g, b, 1);
			pointLight[i].SetDropOff(1.1f);
			pointLight[i].SetPow(1.5f);

			float x = static_cast<float>(rand() % MAX_LIGHT_DISTANCE + 2) * (rand() % 2 ? 1 : -1);
			float y = static_cast<float>(rand() % MAX_LIGHT_DISTANCE + 2) * (rand() % 2 ? 1 : -1);
			float z = static_cast<float>(rand() % MAX_LIGHT_DISTANCE + 2) * (rand() % 2 ? 1 : -1);

			float scl = static_cast<float>(rand() % 13 );
			pointLight[i].SetPosition(x, y, z);
			pointLight[i].SetIntensity(scl);

		}
		
		std::vector<Drawable> d(NUMBER_OF_DRAWABLES);
		
		Drawable dynamicDrawable;
		dynamicDrawable.SetTexture(&t[0]);
		dynamicDrawable.SetNormal(&t[1]);
		dynamicDrawable.SetMetallic(&t[2]);
		//dynamicDrawable.SetAsStatic();
		dynamicDrawable.SetMesh(m);
		dynamicDrawable.SetScale(10, 10, 10);

		for (int i = 0; i < NUMBER_OF_DRAWABLES; i++)
		{
			d[i].SetTexture(&t2[0]);
			d[i].SetNormal(&t2[1]);
			d[i].SetMetallic(&t2[2]);
			d[i].SetAsStatic();
			d[i].SetMesh(m);
		
			float x = (FLOAT)(rand() % MAX_DISTANCE) * (rand() % 2 ? 1 : -1);
			float y = (FLOAT)(rand() % MAX_DISTANCE) * (rand() % 2 ? 1 : -1);
			float z = (FLOAT)(rand() % MAX_DISTANCE) * (rand() % 2 ? 1 : -1);
		
			float scl = (FLOAT)(rand() % 3 + 1);
		
			d[i].SetPosition(x, y, z);
			d[i].SetScale(scl, scl, scl);
			d[i].SetScale(1,1,1);
		}

		d[0].SetPosition(0, 0, 50);
		d[0].SetScale(100, 100, 1);

		d[1].SetPosition(0, 0, -50);
		d[1].SetScale(100, 100, 1);

		d[2].SetPosition(0,-50,0);
		d[2].SetScale(100,1,100);

		d[3].SetPosition(0, 50, 0);
		d[3].SetScale(100, 1, 100);

		d[4].SetPosition(50, 0, 0);
		d[4].SetScale(1, 100, 100);

		d[5].SetPosition(-50, 0, 0);
		d[5].SetScale(1, 100, 100);

		d[6].SetPosition(-25, -52.5f, -25);
		d[6].SetScale(100, 100, 100);
		d[6].SetMesh(bunny);
		d[6].SetTexture(&bunnyTexture[0]);
		d[6].SetNormal(&bunnyTexture[1]);
		d[6].SetMetallic(&bunnyTexture[2]);
		
		d[7].SetPosition(-05, -40.5f, -25);
		d[7].SetScale(10, 10, 10);
		d[7].SetMesh(m);
		d[7].SetTexture(&t[0]);
		d[7].SetNormal(&t[1]);
		d[7].SetMetallic(&t[2]);

		d[8].SetPosition(25, -40.5f, -25);
		d[8].SetScale(10, 10, 10);
		d[8].SetMesh(m2);
		d[8].SetTexture(&bunnyTexture[0]);
		d[8].SetNormal(&bunnyTexture[1]);
		d[8].SetMetallic(&bunnyTexture[2]);

		float rot = 0;
		timer.Start();

		Window * wnd = Window::GetInstance();

		POINT mpStart = wnd->GetWindowSize();
		DirectX::XMFLOAT2 mousePosLastFrame = { (float)mpStart.x, (float)mpStart.y };
		mousePosLastFrame.x /= 2;
		mousePosLastFrame.y /= 2;

		wnd->ResetMouse();



		DirectionalLight light;
		light.SetDirection(1, -1, 0);
		light.SetIntensity(0.5f);
		while (core->Running())
		{
			POINT mp = wnd->GetMousePosition();
			DirectX::XMFLOAT2 mouseThisFrame = { (float)mp.x, (float)mp.y };
			static const float MOVE_SPEED = 5.0f;
			static const float SPRINT_SPEED = 50.0f;
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
				if (wnd->IsKeyPressed(Input::C))
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


			//light.Queue();
		
			for (int i = 0; i < NUMBER_OF_DRAWABLES; i++)
			{
				d[i].Update();
				d[i].Draw();
			}

			rot += 1.0f * (FLOAT)dt;

			static float mover = 0.0f;
			static const float SPEED = 1.0f;

			dynamicDrawable.SetPosition(0, sin(mover) * 10.0f, 0.0f);
			dynamicDrawable.SetRotation(0, mover, 0.0f);
			dynamicDrawable.SetScale((cos(mover) * 5.0f) + 10.0f, (sin(mover) * 5.0f) + 10.0f, (cos(mover) * 5.0f) + 10.0f);

			mover += SPEED * dt;

			if (mover >= DirectX::XM_2PI)
				mover = 0;

			dynamicDrawable.Update();
			dynamicDrawable.Draw();

			for (UINT i = 0; i < NUMBER_OF_LIGHTS; i++)
			{
				pointLight[i].Queue();
			}
			
			if (FAILED(core->Flush()))
			{
				DEBUG::CreateError("LOL");
				break;
			}

			
		}
		core->ClearGPU();
		m->Release();
		delete m;
		m2->Release();
		delete m2;
		bunny->Release();
		delete bunny;
		for (UINT i = 0; i < 3; i++)
		{
			t[i].Release();
			t2[i].Release();
			bunnyTexture[i].Release();
		}
		delete[] t;
		delete[] t2;
		delete[] bunnyTexture;
		delete[] pointLight;
	}
	core->Release();
	delete core;
	 
	return 0;
}

void printFrameTime(double dt)
{
	static int			counter = 0;
	static const int	MAX_DATA = 10;
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