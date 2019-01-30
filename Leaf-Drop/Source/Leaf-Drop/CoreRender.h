#pragma once
#include <d3d12.h>
#include <Windows.h>

class CoreRender
{
private:
	CoreRender();
	~CoreRender();
public:

	CoreRender * GetInstance();

	HRESULT Init();

private:



};

