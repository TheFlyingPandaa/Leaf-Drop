#pragma once
class Fence
{
public:
	Fence();
	~Fence();
	HRESULT CreateFence(const std::wstring & name);

	
	HRESULT Signal(ID3D12CommandQueue * commandQueue);
	HRESULT Wait(ID3D12CommandQueue * commandQueue);
	bool Done();

	void Release();
private:
	ID3D12Fence *			m_fence = nullptr;
	HANDLE					m_fenceEvent = nullptr;
	UINT64 					m_fenceValue = 0;

};

