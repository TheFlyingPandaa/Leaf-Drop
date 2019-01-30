#include "CorePCH.h"
#include "Core.h"



Core::Core()
{
	PRINT("LOL");
}

Core::~Core()
{
	m_coreRenderer->Release();
}

HRESULT Core::Init(HINSTANCE hInstance)
{
	m_window = Window::GetInstance();
	m_window->Create(hInstance, 10, 800, 800);

	m_coreRenderer = CoreRender::GetInstance();
	m_coreRenderer->Init();

	return E_NOTIMPL;
}
