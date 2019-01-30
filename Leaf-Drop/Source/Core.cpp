#include "CorePCH.h"
#include "Core.h"



Core::Core()
{
	PRINT("LOL");
}

Core::~Core()
{
}

HRESULT Core::Init(HINSTANCE hInstance)
{
	m_window = Window::GetInstance();
	m_window->Create(hInstance, 10, 800, 800);

	return E_NOTIMPL;
}
