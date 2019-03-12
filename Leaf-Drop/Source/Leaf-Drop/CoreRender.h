#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>

#include <Windows.h>

#include "Extern/Extern.h"

#include "Passes/GeometryPass.h"
#include "Passes/DeferredPass.h"
#include "Passes/ComputePass.h"
#include "Passes/PrePass.h"
#include "Passes/UpdatePass.h"
#include "Passes/RayDefinePass.h"
#include "Wrappers/CpuTimer.h"

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
	ID3D12DescriptorHeap * GetGPUDescriptorHeap() const;
	ID3D12CommandQueue * GetCopyQueue() const;

	HRESULT BeginCopy();
	HRESULT EndCopy();
	ID3D12GraphicsCommandList * GetCopyCommandList() const;
	
	const UINT & GetRTVDescriptorHeapSize() const;

	const UINT & GetFrameIndex() const;

	ID3D12DescriptorHeap *	GetCPUDescriptorHeap() const;
	const SIZE_T &			GetCurrentResourceIndex() const;
	const SIZE_T &			GetResourceDescriptorHeapSize() const;
	void					IterateResourceIndex(const UINT & arraySize = 1);

	PrePass		 * GetPrePass() const;
	UpdatePass	 * GetUpdatePass() const;
	GeometryPass * GetGeometryPass() const;
	DeferredPass * GetDeferredPass() const;
	ComputePass	 * GetComputePass() const;
	RayDefinePass * GetRayDefinePass() const;

	HRESULT OpenCommandList();
	HRESULT ExecuteCommandList();

	HRESULT Flush();

	void ClearGPU();

	void SetResourceDescriptorHeap(ID3D12GraphicsCommandList * commandList) const;	
	const SIZE_T & CopyToGPUDescriptorHeap(const D3D12_CPU_DESCRIPTOR_HANDLE & handle, const UINT & numDescriptors);

private:

	ID3D12Device *				m_device = nullptr;
	IDXGISwapChain3 *			m_swapChain = nullptr;
	ID3D12CommandQueue *		m_commandQueue = nullptr;
	ID3D12DescriptorHeap *		m_rtvDescriptorHeap = nullptr;

	ID3D12DescriptorHeap *		m_gpuDescriptorHeap[FRAME_BUFFER_COUNT] = { nullptr };
	
	ID3D12CommandQueue	*		m_copyQueue = nullptr;
	ID3D12CommandAllocator *	m_copyCommandAllocator[FRAME_BUFFER_COUNT] = { nullptr };
	ID3D12GraphicsCommandList * m_copyCommandList[FRAME_BUFFER_COUNT] = { nullptr };


	ID3D12DescriptorHeap *		m_cpuDescriptorHeap = nullptr;
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

	SIZE_T m_gpuOffset[FRAME_BUFFER_COUNT] = {0};

	PrePass * m_prePass = nullptr;
	UpdatePass * m_updatePass = nullptr;
	GeometryPass * m_geometryPass = nullptr;
	DeferredPass * m_deferredPass = nullptr;
	ComputePass * m_computePass = nullptr;
	RayDefinePass * m_rayDefinePass = nullptr;

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
	HRESULT _CreateCPUDescriptorHeap();
	HRESULT _CreateCopyQueue();

	void _Clear();


private: //  DEBUG
	ID3D12Debug * m_debugLayer = nullptr;

};

