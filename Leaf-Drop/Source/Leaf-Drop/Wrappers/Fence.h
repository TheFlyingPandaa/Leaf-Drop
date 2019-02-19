#pragma once
class Fence
{
public:
	Fence();
	~Fence();
	HRESULT CreateFence(ID3D12CommandQueue * commandQueue);
	HRESULT WaitForFinnishExecution();

	void Release();
private:
	ID3D12Fence *			m_fence[FRAME_BUFFER_COUNT]{ nullptr };
	HANDLE					m_fenceEvent = nullptr;
	UINT64 					m_fenceValue[FRAME_BUFFER_COUNT]{ 0 };
	ID3D12CommandQueue *	m_ptrCommandQueue = nullptr;

};

