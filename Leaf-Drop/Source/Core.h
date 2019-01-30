#pragma once
#include "Window/Window.h"

class Core
{
private:
	Window * m_window;

public:
	Core();
	~Core();

	HRESULT Init(HINSTANCE hInstance);

};

