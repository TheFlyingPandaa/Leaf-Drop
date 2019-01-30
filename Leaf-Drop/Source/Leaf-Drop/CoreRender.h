#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>

#include <Windows.h>

#include "Extern/Extern.h"

class Window;

class CoreRender
{
private:
	CoreRender();
	~CoreRender();
public:

	static CoreRender * GetInstance();

	HRESULT Init();

	void Release();

private:

	ID3D12Device *				m_device = nullptr;
	IDXGISwapChain3 *			m_swapChain = nullptr;
	ID3D12CommandQueue *		m_commandQueue = nullptr;
	ID3D12GraphicsCommandList * m_commandList = nullptr;
	ID3D12DescriptorHeap *		m_rtvDescriptorHeap = nullptr;

	ID3D12CommandAllocator *	m_commandAllocator[FRAME_BUFFER_COUNT]{ nullptr };
	ID3D12Resource *			m_renderTargets[FRAME_BUFFER_COUNT]{ nullptr };
	ID3D12Fence *				m_fence[FRAME_BUFFER_COUNT]{ nullptr };
	UINT64 						m_fenceValue[FRAME_BUFFER_COUNT]{ 0 };

	UINT m_frameIndex = 0;
	UINT m_rtvDescriptorSize = 0;

	HANDLE m_fenceEvent = nullptr;

	Window * m_windowPtr = nullptr;


private:
	HRESULT _CheckD3D12Support(IDXGIAdapter1 *& adapter, IDXGIFactory4 *& dxgiFactory) const;
	HRESULT _CreateCommandQueue();
	HRESULT _CreateSwapChain(IDXGIFactory4 * dxgiFactory);
	HRESULT _CreateRenderTargetDescriptorHeap();
	HRESULT _CreateCommandAllocators();
	HRESULT _CreateCommandList();
	HRESULT _CreateFenceAndFenceEvent();

private: //  DEBUG
	ID3D12Debug * m_debugLayer = nullptr;

};

