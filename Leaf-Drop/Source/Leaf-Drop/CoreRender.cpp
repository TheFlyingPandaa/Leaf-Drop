#include "CorePCH.h"
#include "CoreRender.h"

#include "../Window/Window.h"

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

	//m_geometryPass = new GeometryPass();
	//if (FAILED(hr = m_geometryPass->Init()))
	//{
	//	return DEBUG::CreateError(hr);
	//}
	return hr;
}

void CoreRender::Release()
{
	SAFE_RELEASE(m_swapChain);
	SAFE_RELEASE(m_commandQueue);
	SAFE_RELEASE(m_commandList);
	SAFE_RELEASE(m_rtvDescriptorHeap);
	
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_commandAllocator[i]);
		SAFE_RELEASE(m_renderTargets[i]);
		SAFE_RELEASE(m_fence[i]);
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

const UINT & CoreRender::GetFrameIndex() const
{
	return this->m_frameIndex;
}

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
	return hr;
}

HRESULT CoreRender::_Flush()
{
	HRESULT hr = 0;

	if (FAILED(hr = _UpdatePipeline()))
	{
		return hr;
	}

	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (FAILED(hr = m_commandQueue->Signal(m_fence[m_frameIndex], m_fenceValue[m_frameIndex])))
	{
		return hr;
	}
	return hr;
}

HRESULT CoreRender::_UpdatePipeline()
{
	HRESULT hr = 0;

	if (FAILED(hr = _waitForPreviousFrame()))
	{
		return hr;
	}

	if (FAILED(hr = m_commandAllocator[m_frameIndex]->Reset()))
	{
		return hr;
	}
	if (FAILED(hr = m_commandList->Reset(m_commandAllocator[m_frameIndex], nullptr)))
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

		m_commandList->ResourceBarrier(1, &barrier);
	}


	const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = 
		{ m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + 
		m_frameIndex * 
		m_rtvDescriptorSize };

	m_commandList->OMSetRenderTargets(1, &rtvHandle, NULL, nullptr);
	static float clearColor[] = { 1.0f, 0.0f, 1.0f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	

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

		m_commandList->ResourceBarrier(1, &barrier);
	}

	m_commandList->Close();
	return hr;

}

HRESULT CoreRender::_waitForPreviousFrame()
{
	HRESULT hr = 0;

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	if (m_fence[m_frameIndex]->GetCompletedValue() < m_fenceValue[m_frameIndex])
	{
		if (FAILED(hr = m_fence[m_frameIndex]->SetEventOnCompletion(m_fenceValue[m_frameIndex], m_fenceEvent)))
		{
			return hr;
		}
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_fenceValue[m_frameIndex]++;

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

	UINT2 windowSize = m_windowPtr->GetWindowSize();

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

	if (SUCCEEDED(hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0], nullptr, IID_PPV_ARGS(&m_commandList))))
	{
		SET_NAME(m_commandList, L"Default CommandList");
		m_commandList->Close();
	}
	return hr;
}

HRESULT CoreRender::_CreateFenceAndFenceEvent()
{
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence[i]))))
		{
			break;
		}
		m_fenceValue[i] = 0;
	}

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (nullptr == m_fenceEvent)
		return E_FAIL;

	return hr;
}
