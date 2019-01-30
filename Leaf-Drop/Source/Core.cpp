#include "CorePCH.h"
#include "Core.h"

#include <EASTL/vector.h>

Core::Core()
{
	PRINT("LOL");
}

Core::~Core()
{
	m_coreRenderer->Release();
	m_window->Close();
}

HRESULT Core::Init(HINSTANCE hInstance)
{
	m_window = Window::GetInstance();
	if (!m_window->Create(hInstance, 10, 800, 800))
	{
		return E_ABORT;
	}

	m_coreRenderer = CoreRender::GetInstance();
	HRESULT hr = 0;
	if (FAILED(hr = m_coreRenderer->Init()))
	{
		return hr;
	}

	return S_OK;
}

BOOL Core::Running() const
{
	return m_window->IsOpen();
}
