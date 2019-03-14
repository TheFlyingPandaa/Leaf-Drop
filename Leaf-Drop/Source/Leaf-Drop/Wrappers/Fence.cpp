#include "CorePCH.h"
#include "Fence.h"


Fence::Fence()
{
}


Fence::~Fence()
{
}

HRESULT Fence::CreateFence(const std::wstring & name)
{
	HRESULT hr = 0;


	CoreRender * p_coreRender = CoreRender::GetInstance();


	if (SUCCEEDED(p_coreRender->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
	{
		m_fence->SetName(name.c_str());
		m_fenceValue = 0;
	}
	
	m_fenceEvent = CreateEvent(0, false, false, L"Geometry fence");
	return hr;
}

//HRESULT Fence::WaitForFinnishExecution()
//{
//	CoreRender * p_coreRender = CoreRender::GetInstance();
//	const UINT frameIndex = p_coreRender->GetFrameIndex();
//
//	HRESULT hr = 0;
//
//
//	// TODO :: wait for queue (->wait());
//	//if (SUCCEEDED(hr = m_ptrCommandQueue->Signal(m_fence[frameIndex], m_fenceValue[frameIndex])))
//	//{
//	//	if (m_fence[frameIndex]->GetCompletedValue() < m_fenceValue[frameIndex])
//	//	{
//	//		if (SUCCEEDED(m_fence[frameIndex]->SetEventOnCompletion(m_fenceValue[frameIndex], m_fenceEvent)))
//	//		{
//	//			//if (SUCCEEDED(hr = m_ptrCommandQueue->Wait(m_fence[frameIndex], m_fenceValue[frameIndex])))
//	//			//{
//	//			//}
//	//			WaitForSingleObject(m_fenceEvent, INFINITE); 
//	//		}
//	//	}
//	//	m_fenceValue[frameIndex]++;
//	//}
//	return hr;
//}

HRESULT Fence::Signal(ID3D12CommandQueue * commandQueue)
{
	m_fenceValue++;
	HRESULT hr = commandQueue->Signal(m_fence, m_fenceValue);
	return hr;
}

HRESULT Fence::Wait(ID3D12CommandQueue * commandQueue)
{
	HRESULT hr = 0;

	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		if (SUCCEEDED(hr = commandQueue->Wait(m_fence, m_fenceValue)))
		{
			//if (SUCCEEDED(hr = m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent)))
			//{
			//}
		}
	}		

	return hr;
}

void Fence::Release()
{
	SAFE_RELEASE(m_fence);	
}
