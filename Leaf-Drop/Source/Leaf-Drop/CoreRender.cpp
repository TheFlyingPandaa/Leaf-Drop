#include "CorePCH.h"
#include "CoreRender.h"

#include "../Window/Window.h"

#define MAX_DESCRIPTOR_HEAP_SIZE 1000

namespace DEBUG
{
	HRESULT CreateError(const HRESULT& hr)
	{
		const _com_error err(hr);
		return CreateError(err.ErrorMessage());
	}

	HRESULT CreateError(const std::string& errorMsg)
	{
		return CreateError(std::wstring(errorMsg.begin(), errorMsg.end()));
	}

	HRESULT CreateError(const std::wstring& errorMsg)
	{
		return CreateError(LPCWSTR(errorMsg.c_str()));
	}

	HRESULT CreateError(const LPCWSTR& errorMsg)
	{
		MessageBoxW(nullptr, errorMsg,
			L"Error", MB_OK | MB_ICONERROR);
		return E_FAIL;
	}
}

CoreRender::CoreRender()
{
}

CoreRender::~CoreRender()
{
	delete m_geometryPass;
	delete m_deferredPass;
	delete m_computePass;
	delete m_prePass;
	delete m_updatePass;
	delete m_rayDefinePass;


}

CoreRender * CoreRender::GetInstance()
{
	static CoreRender coreRenderer;
	return &coreRenderer;
}

HRESULT CoreRender::Init()
{
	m_windowPtr = Window::GetInstance();
	HRESULT hr = 0;
	
	IDXGIAdapter1 * adapter = nullptr;
	IDXGIFactory4 * factory = nullptr;

	if (FAILED(hr = _CheckD3D12Support(adapter, factory)))
	{
		SAFE_RELEASE(adapter);
		return DEBUG::CreateError(hr);
	}
#ifdef _DEBUG
	if (FAILED(hr = D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugLayer))))
	{		
		return DEBUG::CreateError(hr);
	}
	m_debugLayer->EnableDebugLayer();
#endif

	if (FAILED(hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device))))
	{
		SAFE_RELEASE(adapter);
		return DEBUG::CreateError(hr);
	}

	SAFE_RELEASE(adapter);

	if (FAILED(hr = _CreateCommandQueue()))
	{
		SAFE_RELEASE(factory);
		return DEBUG::CreateError(hr);
	}
	
	if (FAILED(hr = _CreateSwapChain(factory)))
	{
		SAFE_RELEASE(factory);
		return DEBUG::CreateError(hr);
	}
	SAFE_RELEASE(factory);
	

	if (FAILED(hr = _CreateRenderTargetDescriptorHeap()))
	{
		return DEBUG::CreateError(hr);
	}

	if (FAILED(hr = _CreateCommandAllocators()))
	{
		return DEBUG::CreateError(hr);
	}

	if (FAILED(hr = _CreateCommandList()))
	{
		return DEBUG::CreateError(hr);
	}

	if (FAILED(hr = _CreateFenceAndFenceEvent()))
	{
		return DEBUG::CreateError(hr);
	}

	if (FAILED(hr = _CreateResourceDescriptorHeap()))
	{
		return DEBUG::CreateError(hr);
	}

	if (FAILED(hr = _CreateCPUDescriptorHeap()))
	{
		return DEBUG::CreateError(hr);
	}

	if (FAILED(hr = _CreateCopyQueue()))
	{
		return DEBUG::CreateError(hr);
	}

	m_prePass = new PrePass();
	if (FAILED(hr = m_prePass->Init()))
	{
		return DEBUG::CreateError(hr);
	}

	m_updatePass = new UpdatePass();
	if (FAILED(hr = m_updatePass->Init()))
	{
		return DEBUG::CreateError(hr);
	}

	m_geometryPass = new GeometryPass();
	if (FAILED(hr = m_geometryPass->Init()))
	{
		return DEBUG::CreateError(hr);
	}

	m_deferredPass = new DeferredPass();
	if (FAILED(hr = m_deferredPass->Init()))
	{
		return DEBUG::CreateError(hr);
	}



	m_computePass = new ComputePass();
	if (FAILED(hr = m_computePass->Init()))
	{
		return DEBUG::CreateError(hr);
	}

	m_rayDefinePass = new RayDefinePass();
	if (FAILED(hr = m_rayDefinePass->Init()))
	{
		return DEBUG::CreateError(hr);
	}
	

	for (UINT i = 0; i < 6; i++)
	{
		if (i == UPDATE)
		{
			if (FAILED(hr = StefanTimer[i].Init(NUM_FRAMES, D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP )))
				return DEBUG::CreateError(hr);
		}
		else
			if (FAILED(hr = StefanTimer[i].Init(NUM_FRAMES)))
				return DEBUG::CreateError(hr);
	}

	return hr;
}

void CoreRender::Release()
{

	m_prePass->KillThread();
	m_updatePass->KillThread();
	m_geometryPass->KillThread();
	m_computePass->KillThread();
	m_rayDefinePass->KillThread();


	m_prePass->Release();
	m_updatePass->Release();
	m_geometryPass->Release();
	m_deferredPass->Release();
	m_computePass->Release();
	m_rayDefinePass->Release();

	TextureAtlas::GetInstance()->Release();


	SAFE_RELEASE(m_swapChain);
	SAFE_RELEASE(m_commandQueue);
	SAFE_RELEASE(m_rtvDescriptorHeap);
	SAFE_RELEASE(m_cpuDescriptorHeap);
	SAFE_RELEASE(m_copyQueue);
	SAFE_RELEASE(m_fence);
	
	FOR_FRAME
	{
		SAFE_RELEASE(m_gpuDescriptorHeap[i]);
		SAFE_RELEASE(m_commandAllocator[i]);
		SAFE_RELEASE(m_renderTargets[i]);
		SAFE_RELEASE(m_commandList[i]);
		SAFE_RELEASE(m_copyCommandAllocator[i]);
		SAFE_RELEASE(m_copyCommandList[i]);		
	}
	for (UINT i = 0; i < 6; i++)
	{
		m_passFences[i].Release();
		StefanTimer[i].Release();
	}

#ifdef _DEBUG
	if (m_device)
	{
		if (m_device->Release())
		{
			ID3D12DebugDevice * dbgDevice = nullptr;
			if (SUCCEEDED(m_device->QueryInterface(IID_PPV_ARGS(&dbgDevice))))
			{
				dbgDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
				SAFE_RELEASE(dbgDevice);
			}
		}
		m_device = nullptr;
	}
#endif
	SAFE_RELEASE(m_device);
}

ID3D12Device * CoreRender::GetDevice() const
{
	return this->m_device;
}

ID3D12CommandQueue * CoreRender::GetCommandQueue() const
{
	return this->m_commandQueue;
}

ID3D12GraphicsCommandList *const* CoreRender::GetCommandList() const
{
	return this->m_commandList;
}

IDXGISwapChain3 * CoreRender::GetSwapChain() const
{
	return this->m_swapChain;
}

ID3D12DescriptorHeap * CoreRender::GetRTVDescriptorHeap() const
{
	return m_rtvDescriptorHeap;
}

ID3D12DescriptorHeap * CoreRender::GetGPUDescriptorHeap() const
{
	return m_gpuDescriptorHeap[m_frameIndex];
}

ID3D12CommandQueue * CoreRender::GetCopyQueue() const
{
	return m_copyQueue;
}

HRESULT CoreRender::BeginCopy()
{
	HRESULT hr = 0; 
	if (FAILED(hr = m_copyCommandAllocator[m_frameIndex]->Reset()))
	{
		return hr;
	}
	if (FAILED(hr = m_copyCommandList[m_frameIndex]->Reset(m_copyCommandAllocator[m_frameIndex], nullptr)))
	{
		return hr;
	}
	return hr;
}

HRESULT CoreRender::EndCopy()
{
	HRESULT hr = 0;

	if (FAILED(hr = m_copyCommandList[m_frameIndex]->Close()))
		return hr;
	ID3D12CommandList* ppCommandLists[] = { m_copyCommandList[m_frameIndex] };
	m_copyQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	//if (FAILED(hr = m_commandQueue->Signal(m_fence[m_frameIndex], m_fenceValue[m_frameIndex])))
	//{
	//	return hr;
	//}
	//TODO:: possible fence bobby please fix
	return hr;
}

ID3D12GraphicsCommandList * CoreRender::GetCopyCommandList() const
{
	return m_copyCommandList[m_frameIndex];
}

const UINT & CoreRender::GetRTVDescriptorHeapSize() const
{
	return m_rtvDescriptorSize;
}

const UINT & CoreRender::GetFrameIndex() const
{
	return this->m_frameIndex;
}

ID3D12DescriptorHeap * CoreRender::GetCPUDescriptorHeap() const
{
	return m_cpuDescriptorHeap;
}

const SIZE_T & CoreRender::GetCurrentResourceIndex() const
{
	return m_currentResourceIndex;
}

const SIZE_T & CoreRender::GetResourceDescriptorHeapSize() const
{
	return m_resourceDescriptorHeapSize;
}

void CoreRender::IterateResourceIndex(const UINT & arraySize)
{
	m_currentResourceIndex+= arraySize;
}

PrePass * CoreRender::GetPrePass() const
{
	return m_prePass;
}

UpdatePass * CoreRender::GetUpdatePass() const
{
	return m_updatePass;
}

GeometryPass * CoreRender::GetGeometryPass() const
{
	return m_geometryPass;
}

DeferredPass * CoreRender::GetDeferredPass() const
{
	return this->m_deferredPass;
}

ComputePass * CoreRender::GetComputePass() const
{
	return m_computePass;
}

RayDefinePass * CoreRender::GetRayDefinePass() const
{
	return m_rayDefinePass;
}

HRESULT CoreRender::OpenCommandList()
{
	HRESULT hr = 0;
	if (FAILED(hr = m_commandAllocator[m_frameIndex]->Reset()))
	{
		return hr;
	}
	if (FAILED(hr = m_commandList[m_frameIndex]->Reset(m_commandAllocator[m_frameIndex], nullptr)))
	{
		return hr;
	}
	return hr;
}

HRESULT CoreRender::ExecuteCommandList()
{
	HRESULT hr = 0;
	ID3D12CommandList* ppCommandLists[] = { m_commandList[m_frameIndex] };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (FAILED(hr = m_commandQueue->Signal(m_fence, m_fenceValue)))
	{
		return hr;
	}
	return hr;
}

#include "Objects/Texture.h"

HRESULT CoreRender::Flush()
{
	HRESULT hr = 0;

	if (FAILED(hr = this->_Flush()))
	{
		return DEBUG::CreateError(hr);
	}

	if (FAILED(hr = m_swapChain->Present(0, 0)))
	{
		return DEBUG::CreateError(hr);
	}

	if (RENDER_TO_JPEG || RENDER_TO_BMP)
	{
		static int frameCounter = 0;
		ID3D12Resource * buffer = nullptr;

		hr = m_swapChain->GetBuffer(m_swapChain->GetCurrentBackBufferIndex(), IID_PPV_ARGS(&buffer));
		if (SUCCEEDED(hr))
		{
			if (RENDER_TO_BMP)
			{
				hr = Texture::SaveToBMP(m_commandQueue, buffer, frameCounter++);
			}
			else
			{
				hr = Texture::SaveToJPEG(m_commandQueue, buffer, frameCounter++);
			}
			if (buffer)
				buffer->Release();
			if (FAILED(hr))
				return DEBUG::CreateError(hr);

		}
		else
		{
			return DEBUG::CreateError(hr);
		}

	}
	

	_Clear();
	return hr;
}

void CoreRender::ClearGPU()
{
	
	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		if (FAILED(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent)))
		{

		}
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_fenceValue++;
	
}

void CoreRender::SetResourceDescriptorHeap(ID3D12GraphicsCommandList * commandList) const
{
	ID3D12DescriptorHeap * heaps[]{ m_gpuDescriptorHeap[m_frameIndex] };
	commandList->SetDescriptorHeaps(_countof(heaps), heaps);
}

const SIZE_T & CoreRender::CopyToGPUDescriptorHeap(const D3D12_CPU_DESCRIPTOR_HANDLE & handle, const UINT & numDescriptors)
{
	m_gpuOffset[m_frameIndex] += m_resourceDescriptorHeapSize * numDescriptors;
	m_device->CopyDescriptorsSimple(numDescriptors,
		{ m_gpuDescriptorHeap[m_frameIndex]->GetCPUDescriptorHandleForHeapStart().ptr + m_gpuOffset[m_frameIndex] },
		handle,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	return m_gpuOffset[m_frameIndex];
}

Fence * CoreRender::GetFence(const UINT & index)
{
	return &m_passFences[index];
}

HRESULT CoreRender::_Flush()
{
	HRESULT hr = 0;

	if (FAILED(hr = _UpdatePipeline()))
	{
		return hr;
	}
	
	ID3D12CommandList* ppCommandLists[] = { m_commandList[m_frameIndex] };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (FAILED(hr = m_commandQueue->Signal(m_fence, m_fenceValue)))
	{
		return hr;
	}
	
	return hr;
}

#include <iostream>
HRESULT CoreRender::_UpdatePipeline()
{
	HRESULT hr = 0;

	if (FAILED(hr = _waitForPreviousFrame()))
	{
		return hr;
	}

	static bool hasPrinted = false;
	bool Print = true;

	for (int i = 0; i < 6; i++)
	{
		if (StefanTimerCounter[i] < NUM_FRAMES)
		{
			Print = false;
			break;
		}
	}

	if (Print && !hasPrinted)
	{
		std::string arr[] = {
			"_PRE_PASS",
			"_GEOMETRY",
			"_DEFERRED",
			"_UPDATE",
			"_DEFINE",
			"_RAY_TRACING"
		};

		for (int i = 0; i < 6; i++)
		{		
			std::ofstream file;
			file.open(arr[i] + ".txt");

			
			UINT64 queueFreq = StefanTimer[i].GetFreq();
			double timestampToMs = (1.0 / queueFreq) * 1000.0;
			for (int j = 0; j < StefanTimerCounter[i]; j++)
			{
				GPU::Timestamp drawTime;
				drawTime = StefanTimer[i].GetTimestampPair(j);

				UINT64 dt = drawTime.Stop - drawTime.Start;
				double timeInMs = dt * timestampToMs;

				file << drawTime.Start << "\t" << drawTime.Stop << "\t" << timeInMs << "\n";
			}
			file.close();
		}

		hasPrinted = true;
	}

	if (FAILED(hr = m_commandAllocator[m_frameIndex]->Reset()))
	{
		return hr;
	}
	if (FAILED(hr = m_commandList[m_frameIndex]->Reset(m_commandAllocator[m_frameIndex], nullptr)))
	{
		return hr;
	}

	{
		D3D12_RESOURCE_TRANSITION_BARRIER transition;
		transition.pResource = m_renderTargets[m_frameIndex];
		transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		transition.Subresource = 0;

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition = transition;

		m_commandList[m_frameIndex]->ResourceBarrier(1, &barrier);
	}

	struct UINT4
	{
		UINT x, y, z, w;
	};
	
	int counter = 0;
	UINT textureCounter = 0;
	UINT textureIndexOffset = 0;
	UINT4 textureOffset{ 0,0,0,0 };
	TextureAtlas * ptrAtlas = TextureAtlas::GetInstance();

	for (size_t i = 0; i < IRender::p_staticDrawQueue.size(); i++)
	{
		ptrAtlas->CopyBindless(IRender::p_staticDrawQueue[i].DiffuseTexture);
		ptrAtlas->CopyBindless(IRender::p_staticDrawQueue[i].NormalTexture);
		ptrAtlas->CopyBindless(IRender::p_staticDrawQueue[i].MetallicTexture);

		textureOffset.x = (UINT)i * 3;
		textureOffset.y = 3;

		IRender::p_staticDrawQueue[i].TextureOffset = textureOffset.x;
	}



	for (size_t i = 0; i < IRender::p_dynamicDrawQueue.size(); i++)
	{
		ptrAtlas->CopyBindless(IRender::p_dynamicDrawQueue[i].DiffuseTexture);
		ptrAtlas->CopyBindless(IRender::p_dynamicDrawQueue[i].NormalTexture);
		ptrAtlas->CopyBindless(IRender::p_dynamicDrawQueue[i].MetallicTexture);

		textureOffset.x = (UINT)i * 3 + (IRender::p_staticDrawQueue.size() * 3);
		textureOffset.y = 3;

		IRender::p_dynamicDrawQueue[i].TextureOffset = textureOffset.x;
	}

	m_prePass->UpdateThread();
	m_updatePass->UpdateThread();
	
	m_prePass->ThreadJoin();

	m_rayDefinePass->UpdateThread();
	m_geometryPass->UpdateThread();

	m_rayDefinePass->ThreadJoin();
	m_updatePass->ThreadJoin();
		   
	m_computePass->UpdateThread();
	
	m_computePass->ThreadJoin();
	m_geometryPass->ThreadJoin();


	m_deferredPass->Update();
	m_deferredPass->Draw();


	{
		D3D12_RESOURCE_TRANSITION_BARRIER transition;
		transition.pResource = m_renderTargets[m_frameIndex];
		transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		transition.Subresource = 0;

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition = transition;

		m_commandList[m_frameIndex]->ResourceBarrier(1, &barrier);
	}

	m_commandList[m_frameIndex]->Close();	

	return hr;

}

HRESULT CoreRender::_waitForPreviousFrame()
{
	HRESULT hr = 0;
	m_frameIndex = (m_frameIndex + 1) % FRAME_BUFFER_COUNT;
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		if (FAILED(hr = m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent)))
		{
			return hr;
		}
		WaitForSingleObject(m_fenceEvent, INFINITE);
		
	}
	m_fenceValue++;

	return hr;
}

HRESULT CoreRender::_CheckD3D12Support(IDXGIAdapter1 *& adapter, IDXGIFactory4 *& dxgiFactory) const
{
	HRESULT hr = 0;
	if (adapter || dxgiFactory)
		return E_INVALIDARG;

	if (SUCCEEDED(hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))))
	{
		UINT adapterIndex = 0;
		while (hr = dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				adapterIndex++;
				continue;
			}

			if (SUCCEEDED(hr = D3D12CreateDevice(adapter,
				D3D_FEATURE_LEVEL_12_0,
				_uuidof(ID3D12Device),
				nullptr)))
			{
				return hr;
			}
			SAFE_RELEASE(adapter);
		}

	}
	else
	{
		SAFE_RELEASE(dxgiFactory);
	}


	return hr;
}

HRESULT CoreRender::_CreateCommandQueue()
{
	HRESULT hr = 0;

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;
	if (FAILED(hr = this->m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue))))
	{
		SAFE_RELEASE(this->m_commandQueue);
	}
	SET_NAME(m_commandQueue, L"Default CommandQueue");
	return hr;
}

HRESULT CoreRender::_CreateSwapChain(IDXGIFactory4 * dxgiFactory)
{
	HRESULT hr = 0;

	if (!dxgiFactory)
		return E_INVALIDARG;

	POINT windowSize = m_windowPtr->GetWindowSize();

	DXGI_MODE_DESC backBufferDesc = {};
	backBufferDesc.Width = windowSize.x;
	backBufferDesc.Height = windowSize.y;
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = FRAME_BUFFER_COUNT;
	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = m_windowPtr->GetHwnd();
	swapChainDesc.SampleDesc = sampleDesc;
	swapChainDesc.Windowed = !m_windowPtr->IsFullscreen();

	IDXGISwapChain * tmpSwapChain = nullptr;
	if (SUCCEEDED(hr = dxgiFactory->CreateSwapChain(m_commandQueue,
		&swapChainDesc,
		&tmpSwapChain)))
	{
		if (FAILED(hr = tmpSwapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain))))
		{
			SAFE_RELEASE(m_swapChain);
			return E_FAIL;
		}
	}
	SAFE_RELEASE(tmpSwapChain);
	return hr;
}

HRESULT CoreRender::_CreateRenderTargetDescriptorHeap()
{
	HRESULT hr = 0;

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (SUCCEEDED(hr = m_device->CreateDescriptorHeap(
		&rtvHeapDesc,
		IID_PPV_ARGS(&m_rtvDescriptorHeap))))
	{
		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{ m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr };

		for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
		{
			if (FAILED(hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]))))
			{
				break;
			}
			m_device->CreateRenderTargetView(m_renderTargets[i], nullptr, rtvHandle);
			rtvHandle.ptr += m_rtvDescriptorSize;
			SET_NAME(m_renderTargets[i], std::wstring(std::wstring(L"Default RenderTarget") + std::to_wstring(i)).c_str());
		}
		SET_NAME(m_rtvDescriptorHeap, L"RenderTargetViewDescriptorHeap");
	}
	return hr;
}

HRESULT CoreRender::_CreateCommandAllocators()
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i]))))
		{
			break;
		}
		SET_NAME(m_commandAllocator[i], std::wstring(std::wstring(L"Default CommandAllocator") + std::to_wstring(i)).c_str());
	}

	return hr;
}

HRESULT CoreRender::_CreateCommandList()
{
	HRESULT hr = 0;
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (SUCCEEDED(hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0], nullptr, IID_PPV_ARGS(&m_commandList[i]))))
		{
			SET_NAME(m_commandList[i], L"Default CommandList");
			m_commandList[i]->Close();
		}
		else
			return hr;
	}
	return hr;
}

HRESULT CoreRender::_CreateFenceAndFenceEvent()
{
	HRESULT hr = 0;


	if (FAILED(hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
	{
		return hr;
	}
	m_fenceValue = 0;
	

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (nullptr == m_fenceEvent)
		return E_FAIL;

	std::wstring arr[] = {
		L"Fence_PRE_PASS",
		L"Fence_GEOMETRY",
		L"Fence_DEFERRED",
		L"Fence_UPDATE",
		L"Fence_DEFINE",
		L"Fence_RAY_TRACING"
	};



	for (UINT i = 0; i < 6; i++)
	{
		if (FAILED(hr = m_passFences[i].CreateFence(arr[i])))
			return hr;
	}

	return hr;
}

HRESULT CoreRender::_CreateResourceDescriptorHeap()
{
	HRESULT hr = 0;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = MAX_DESCRIPTOR_HEAP_SIZE;

	if (FAILED(hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cpuDescriptorHeap))))
		return hr;

	SET_NAME(m_cpuDescriptorHeap, L"CPU DescriptorHeap");

	m_resourceDescriptorHeapSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	   	 
	return hr;
}

HRESULT CoreRender::_CreateCPUDescriptorHeap()
{
	HRESULT hr = 0;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = MAX_DESCRIPTOR_HEAP_SIZE;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_gpuDescriptorHeap[i]))))
			return hr;
		SET_NAME(m_gpuDescriptorHeap[i], std::wstring(std::wstring(L"GPU DescriptorHeap ") + std::to_wstring(i)).c_str());
	}

	

	return hr;
}

HRESULT CoreRender::_CreateCopyQueue()
{
	HRESULT hr = 0;

	D3D12_COMMAND_QUEUE_DESC desc {};
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

	if (FAILED(hr = m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_copyQueue))))
		return hr;

	FOR_FRAME
	{
		if (FAILED(hr = m_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_COPY, 
			IID_PPV_ARGS(&m_copyCommandAllocator[i]))))
			return hr;
		
		SET_NAME(m_copyCommandAllocator[i], L"Copy");

		if (FAILED(hr = m_device->CreateCommandList(
			0, 
			D3D12_COMMAND_LIST_TYPE_COPY, 
			m_copyCommandAllocator[i], 
			nullptr, 
			IID_PPV_ARGS(&m_copyCommandList[i]))))
			return hr;
		SET_NAME(m_copyCommandList[i], L"Copy");

		m_copyCommandList[i]->Close();
	}

	return hr;
}

void CoreRender::_Clear()
{
	memset(m_gpuOffset, 0, FRAME_BUFFER_COUNT * sizeof(SIZE_T));

	IRender::Clear();
}
