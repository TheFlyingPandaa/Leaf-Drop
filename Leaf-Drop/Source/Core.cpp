#include "CorePCH.h"
#include "Core.h"

#include <EASTL/vector.h>

Core::Core()
{
	
}

Core::~Core()
{
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

HRESULT Core::Flush()
{
	return m_coreRenderer->Flush();
}

BOOL Core::Running() const
{
	if (m_window->IsOpen() == FALSE) {
		m_window->Close();
	}

	return m_window->IsOpen();
}

void Core::ClearGPU()
{
	m_coreRenderer->ClearGPU();
}

void Core::Release()
{
	m_coreRenderer->Release();

}
