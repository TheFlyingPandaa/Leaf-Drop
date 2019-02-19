#include "CorePCH.h"
#include "Fence.h"


Fence::Fence()
{
}


Fence::~Fence()
{
}

HRESULT Fence::CreateFence(ID3D12CommandQueue * commandQueue)
{
	HRESULT hr = 0;

	m_ptrCommandQueue = commandQueue;

	CoreRender * p_coreRender = CoreRender::GetInstance();

	for (UINT i = 0; i < FRAME_BUFFER_COUNT && SUCCEEDED(hr); i++)
	{
		if (SUCCEEDED(p_coreRender->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence[i]))))
		{
			m_fence[i]->SetName(L"Gemory fence");
			m_fenceValue[i] = 0;
		}
	}
	m_fenceEvent = CreateEvent(0, false, false, L"Geometry fence");
	return hr;
}

HRESULT Fence::WaitForFinnishExecution()
{
	CoreRender * p_coreRender = CoreRender::GetInstance();
	const UINT frameIndex = p_coreRender->GetFrameIndex();

	HRESULT hr = 0;

	if (SUCCEEDED(hr = m_ptrCommandQueue->Signal(m_fence[frameIndex], m_fenceValue[frameIndex])))
	{
		if (m_fence[frameIndex]->GetCompletedValue() < m_fenceValue[frameIndex])
		{
			m_fence[frameIndex]->SetEventOnCompletion(m_fenceValue[frameIndex], m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
		m_fenceValue[frameIndex]++;
	}
	return hr;
}

void Fence::Release()
{
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_fence[i]);
	}
}
