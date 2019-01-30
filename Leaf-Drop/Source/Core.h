#pragma once
#include "Window/Window.h"
#include "Leaf-Drop/CoreRender.h"

class Core
{
private:
	Window * m_window = nullptr;
	CoreRender * m_coreRenderer = nullptr;
public:
	Core();
	~Core();

	HRESULT Init(HINSTANCE hInstance);
};

