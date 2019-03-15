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
			
		cam.CreateProjectionMatrix(0.01f, 500.0f);
		cam.SetPosition(0, 0, 0);
		cam.SetAsActiveCamera();

		/*cam.SetPosition(0, -43.2657, 0);
		cam.SetDirection(-0.685973, -0.00897975, -0.727571);*/

		cam.SetPosition(-39.6452, -36.4232, 22.2482);
		cam.SetDirection(0.995644, -0.0104395, -0.0926499);

		cam.Update();

		StaticMesh * m = new StaticMesh();
		StaticMesh * m2 = new StaticMesh();
		StaticMesh * bunny = new StaticMesh();
		StaticMesh * sphere2 = new StaticMesh();

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

		Texture * Mirror2 = new Texture[3];
		Mirror2[0].LoadTexture(L"..\\Assets\\Textures\\Mirror\\Mirror_diffuseOriginal.bmp");
		Mirror2[1].LoadTexture(L"..\\Assets\\Textures\\Mirror\\Mirror_normal.bmp");
		Mirror2[2].LoadTexture(L"..\\Assets\\Textures\\Mirror\\Mirror_metallic.bmp");

		Texture * Roof = new Texture[3];
		Roof[0].LoadTexture(L"..\\Assets\\Textures\\Roof\\Roof_diffuseOriginal.bmp");
		Roof[1].LoadTexture(L"..\\Assets\\Textures\\Wall\\Wall_normal.bmp");
		Roof[2].LoadTexture(L"..\\Assets\\Textures\\Wall\\Wall_metallic.bmp");

		Texture * Wall = new Texture[3];
		Wall[0].LoadTexture(L"..\\Assets\\Textures\\Wall\\Wall_diffuseOriginal.bmp");
		Wall[1].LoadTexture(L"..\\Assets\\Textures\\Wall\\Wall_normal.bmp");
		Wall[2].LoadTexture(L"..\\Assets\\Textures\\Wall\\Wall_metallic.bmp");

		Texture * Wall2 = new Texture[3];
		Wall2[0].LoadTexture(L"..\\Assets\\Textures\\Wall\\Wall2_diffuseOriginal.bmp");
		Wall2[1].LoadTexture(L"..\\Assets\\Textures\\Wall\\Wall_normal.bmp");
		Wall2[2].LoadTexture(L"..\\Assets\\Textures\\Wall\\Wall_metallic.bmp");

		m->LoadMesh("..\\Assets\\Models\\Cube.fbx");
		m2->LoadMesh("..\\Assets\\Models\\Sphere.fbx");
		bunny->LoadMesh("..\\Assets\\Models\\BobTheBunny.fbx");
		sphere2->LoadMesh("..\\Assets\\Models\\NormalSphere.fbx");
		
		const UINT NUMBER_OF_DRAWABLES = 8;
		const UINT NUMBER_OF_DYNAMIC_DRAWABLES = 4;
		const UINT NUMBER_OF_LIGHTS = 200;
		const UINT MAX_DISTANCE = 50;
		const UINT MAX_LIGHT_DISTANCE = 50;

		PointLight * pointLight = new PointLight[NUMBER_OF_LIGHTS];
		for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
		{	
			float r = static_cast<float>(rand() % 10 + 90) / 100.0f;
			float g = static_cast<float>(rand() % 10 + 90) / 100.0f;
			float b = static_cast<float>(rand() % 10 + 90)  / 100.0f;

			pointLight[i].SetColor(r, g, b, 1);
			pointLight[i].SetColor(1, 1, 1, 1);
			pointLight[i].SetDropOff(1.0f);
			pointLight[i].SetPow(2.0f);

			float x = static_cast<float>(rand() % MAX_LIGHT_DISTANCE + 2) * (rand() % 2 ? 1 : -1);
			float y = static_cast<float>(rand() % MAX_LIGHT_DISTANCE + 2) * (rand() % 2 ? 1 : -1);
			float z = static_cast<float>(rand() % MAX_LIGHT_DISTANCE + 2) * (rand() % 2 ? 1 : -1);

			float scl = static_cast<float>(rand() % 40 + 10 );
			pointLight[i].SetPosition(x, y, z);
			pointLight[i].SetIntensity(scl);
		}
		
		std::vector<Drawable> d(NUMBER_OF_DRAWABLES);
		std::vector<Drawable> dynamicD(NUMBER_OF_DYNAMIC_DRAWABLES);
		
		Drawable dynamicDrawable;
		dynamicDrawable.SetTexture(&t[0]);
		dynamicDrawable.SetNormal(&t[1]);
		dynamicDrawable.SetMetallic(&t[2]);
		dynamicDrawable.SetMesh(m);
		dynamicDrawable.SetScale(10, 10, 10);
		dynamicDrawable.SetRotation(0, DirectX::XMConvertToRadians(45), 0);


		for (int i = 0; i < NUMBER_OF_DRAWABLES; i++)
		{
			d[i].SetTexture(&t[0]);
			d[i].SetNormal(&t[0]);
			d[i].SetMetallic(&t[0]);
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
		d[0].SetTexture(&Wall[0]);
		d[0].SetNormal(&Wall[1]);
		d[0].SetMetallic(&Wall[2]);


		d[1].SetPosition(0, 0, -50);
		d[1].SetScale(100, 100, 1);
		d[1].SetTexture(&Wall[0]);
		d[1].SetNormal(&Wall[1]);
		d[1].SetMetallic(&Wall[2]);

		d[2].SetPosition(0,-50,0);
		d[2].SetScale(100,1,100);
		d[2].SetTexture(&Roof[0]);
		d[2].SetNormal(&Roof[1]);
		d[2].SetMetallic(&Roof[2]);

		d[3].SetPosition(0, 50, 0);
		d[3].SetScale(100, 1, 100);
		d[3].SetTexture(&Roof[0]);
		d[3].SetNormal(&Roof[1]);
		d[3].SetMetallic(&Roof[2]);

		d[4].SetPosition(50, 0, 0);
		d[4].SetScale(1, 100, 100);
		d[4].SetTexture(&Wall2[0]);
		d[4].SetNormal(&Wall2[1]);
		d[4].SetMetallic(&Wall2[2]);

		d[5].SetPosition(-50, 0, 0);
		d[5].SetScale(1, 100, 100);
		d[5].SetTexture(&Wall2[0]);
		d[5].SetNormal(&Wall2[1]);
		d[5].SetMetallic(&Wall2[2]);

		d[6].SetPosition(46, -40.0, -1.55212);
		d[6].SetScale(1, 20, 20);
		d[6].SetRotation(0, 0, DirectX::XMConvertToRadians(-20));
		d[6].SetTexture(&t[0]);
		d[6].SetNormal(&t[1]);
		d[6].SetMetallic(&t[2]);

		d[7].SetPosition(-45.9758, -40, 31.1561);
		d[7].SetScale(20, 20, 20);
		d[7].SetRotation(0, 0, 0);
		d[7].SetTexture(&t[0]);
		d[7].SetNormal(&t[1]);
		d[7].SetMetallic(&t[2]);

		dynamicD[0].SetPosition(-42, -52.5f, 42);
		dynamicD[0].SetScale(100, 100, 100);
		dynamicD[0].SetRotation(0, DirectX::XMConvertToRadians(180), 0);
		dynamicD[0].SetMesh(bunny);
		dynamicD[0].SetTexture(&bunnyTexture[0]);
		dynamicD[0].SetNormal(&bunnyTexture[1]);
		dynamicD[0].SetMetallic(&bunnyTexture[2]);
		
		dynamicD[1].SetPosition(30, -30, 30);
		dynamicD[1].SetRotation(0, DirectX::XMConvertToRadians(45), 0);
		dynamicD[1].SetScale(35, 40, 1);
		dynamicD[1].SetMesh(m);
		dynamicD[1].SetTexture(&t[0]);
		dynamicD[1].SetNormal(&t[1]);
		dynamicD[1].SetMetallic(&t[2]);

		dynamicD[2].SetPosition(40, -40.5f, -40);
		dynamicD[2].SetScale(10, 10, 10);
		dynamicD[2].SetMesh(m2);
		dynamicD[2].SetTexture(&bunnyTexture[0]);
		dynamicD[2].SetNormal(&bunnyTexture[1]);
		dynamicD[2].SetMetallic(&bunnyTexture[2]);

		dynamicD[3].SetPosition(-45.9758, 20, -31.1561);
		dynamicD[3].SetScale(10, 10, 10);
		dynamicD[3].SetMesh(m);
		dynamicD[3].SetTexture(&t2[0]);
		dynamicD[3].SetNormal(&t2[1]);
		dynamicD[3].SetMetallic(&t2[2]);


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

			if (Camera::GetActiveCamera() == &cam && false)
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
				cam.Update();
			}

			
			if (wnd->IsKeyPressed(Input::ESCAPE))
				wnd->Close();
					   		
			for (int i = 0; i < NUMBER_OF_DRAWABLES; i++)
			{
				d[i].Update();
				d[i].Draw();
			}

			for (UINT i = 0; i < NUMBER_OF_DYNAMIC_DRAWABLES; i++)
			{
				
				dynamicD[i].Update();
				dynamicD[i].Draw();
			}

			rot += 1.0f * (FLOAT)dt;

			static float mover = 0.0f;
			static const float SPEED = 0.5f;

			dynamicDrawable.SetPosition(-42, -40, -42);
			dynamicDrawable.SetScale(15, 15, 15);

			static float translate = 4.0f;

			dynamicD[1].SetRotation(0, mover, 0.0f);
			dynamicD[3].Translate(translate * dt * 1.5f, -translate * dt, translate * dt);
			dynamicD[3].SetRotation(0, 0, mover);

			mover += SPEED * dt;

			if (mover >= DirectX::XM_2PI)
			{
				mover = 0;
				translate *= -1.0f;
			}

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

		sphere2->Release();
		delete sphere2;

		for (UINT i = 0; i < 3; i++)
		{
			t[i].Release();
			t2[i].Release();
			bunnyTexture[i].Release();
			Wall[i].Release();
			Wall2[i].Release();
			Roof[i].Release();
			Mirror2[i].Release();
		}
		delete[] t;
		delete[] t2;
		delete[] bunnyTexture;
		delete[] pointLight;
		delete[] Wall;
		delete[] Wall2;
		delete[] Roof;
		delete[] Mirror2;
		
	}
	core->Release();
	delete core;
	 
	return 0;
}

void printFrameTime(double dt)
{
	static int			counter = 0;
	static const int	MAX_DATA = 64;
	static double		allTime = 0.0;

	allTime += dt * 1000.0;
	counter++;

	static UINT64 counter2 = 0;

	Window::GetInstance()->SetWindowTitle("Frame: " + std::to_string(counter2++));

	if (counter >= MAX_DATA)
	{
		counter = 0;

		allTime /= MAX_DATA;


		//std::cout << allTime << std::endl;

		allTime = 0.0;
	}
}