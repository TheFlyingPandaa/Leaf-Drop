#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>

#include <Windows.h>

#include "Extern/Extern.h"

#include "../Leaf-Drop/Passes/GeometryPass.h"
#include "../Leaf-Drop/Passes/DeferredPass.h"
#include "../Leaf-Drop/Passes/ComputePass.h"

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

	ID3D12Device * GetDevice() const;
	ID3D12CommandQueue * GetCommandQueue() const;
	ID3D12GraphicsCommandList *const* GetCommandList() const;
	IDXGISwapChain3 * GetSwapChain() const;
	ID3D12DescriptorHeap * GetRTVDescriptorHeap() const;
	const UINT & GetRTVDescriptorHeapSize() const;

	const UINT & GetFrameIndex() const;

	ID3D12DescriptorHeap *	GetResourceDescriptorHeap() const;
	const SIZE_T &			GetCurrentResourceIndex() const;
	const SIZE_T &			GetResourceDescriptorHeapSize() const;
	void					IterateResourceIndex();


	GeometryPass * GetGeometryPass() const;
	DeferredPass * GetDeferredPass() const;

	HRESULT OpenCommandList();
	HRESULT ExecuteCommandList();

	HRESULT Flush();

	void ClearGPU();

	void SetResourceDescriptorHeap(ID3D12GraphicsCommandList * commandList);

private:

	ID3D12Device *				m_device = nullptr;
	IDXGISwapChain3 *			m_swapChain = nullptr;
	ID3D12CommandQueue *		m_commandQueue = nullptr;
	ID3D12DescriptorHeap *		m_rtvDescriptorHeap = nullptr;

	ID3D12DescriptorHeap *		m_resourceDescriptorHeap = nullptr;
	SIZE_T						m_currentResourceIndex = 0;
	SIZE_T						m_resourceDescriptorHeapSize = 0;

	ID3D12GraphicsCommandList * m_commandList[FRAME_BUFFER_COUNT]{ nullptr };
	ID3D12CommandAllocator *	m_commandAllocator[FRAME_BUFFER_COUNT]{ nullptr };
	ID3D12Resource *			m_renderTargets[FRAME_BUFFER_COUNT]{ nullptr };
	ID3D12Fence *				m_fence[FRAME_BUFFER_COUNT]{ nullptr };
	UINT64 						m_fenceValue[FRAME_BUFFER_COUNT]{ 0 };

	UINT m_frameIndex = 0;
	UINT m_rtvDescriptorSize = 0;

	HANDLE m_fenceEvent = nullptr;

	Window * m_windowPtr = nullptr;

	GeometryPass * m_geometryPass = nullptr;
	DeferredPass * m_deferredPass = nullptr;
	ComputePass * m_computePass = nullptr;

	HRESULT _Flush();
	HRESULT _UpdatePipeline();
	HRESULT _waitForPreviousFrame();

private:
	HRESULT _CheckD3D12Support(IDXGIAdapter1 *& adapter, IDXGIFactory4 *& dxgiFactory) const;
	HRESULT _CreateCommandQueue();
	HRESULT _CreateSwapChain(IDXGIFactory4 * dxgiFactory);
	HRESULT _CreateRenderTargetDescriptorHeap();
	HRESULT _CreateCommandAllocators();
	HRESULT _CreateCommandList();
	HRESULT _CreateFenceAndFenceEvent();
	HRESULT _CreateResourceDescriptorHeap();

	void _Clear();


private: //  DEBUG
	ID3D12Debug * m_debugLayer = nullptr;

};

