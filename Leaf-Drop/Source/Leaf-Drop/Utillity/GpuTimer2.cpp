#include "CorePCH.h"
#include "GpuTimer2.h"



namespace GPU
{
	GpuTimer2::GpuTimer2()
	{
	}

	GpuTimer2::~GpuTimer2()
	{
	}

	HRESULT GpuTimer2::Init(UINT numberOfTimers, D3D12_QUERY_HEAP_TYPE heapType)
	{
		m_pDevice = CoreRender::GetInstance()->GetDevice();
		HRESULT hr = 0;

		m_timerCount = numberOfTimers;

		D3D12_QUERY_HEAP_DESC queryHeapDesc;
		queryHeapDesc.Type = heapType;
		queryHeapDesc.NodeMask = 0;
		queryHeapDesc.Count = m_timerCount * 2;

		if (SUCCEEDED(hr = m_pDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&m_queryHeap))))
		{
			D3D12_RESOURCE_DESC resouceDesc;
			ZeroMemory(&resouceDesc, sizeof(resouceDesc));
			resouceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resouceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			resouceDesc.Width = sizeof(Timestamp) * m_timerCount;
			resouceDesc.Height = 1;
			resouceDesc.DepthOrArraySize = 1;
			resouceDesc.MipLevels = 1;
			resouceDesc.Format = DXGI_FORMAT_UNKNOWN;
			resouceDesc.SampleDesc.Count = 1;
			resouceDesc.SampleDesc.Quality = 0;
			resouceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resouceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			D3D12_HEAP_PROPERTIES heapProp = {};
			heapProp.Type = D3D12_HEAP_TYPE_READBACK;
			heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProp.CreationNodeMask = 1;
			heapProp.VisibleNodeMask = 1;

			if (SUCCEEDED(hr = m_pDevice->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resouceDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_queryResourceCPU))
			))
			{
				m_queryResourceCPU->SetName(L"queryResourceCPU_");
			}

			heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
			if (SUCCEEDED(hr = m_pDevice->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&resouceDesc,
				D3D12_RESOURCE_STATE_COPY_SOURCE,
				nullptr,
				IID_PPV_ARGS(&m_queryResourceGPU))
			))
			{
				m_queryResourceGPU->SetName(L"queryResourceGPU_");
			}
		}

		return hr;
	}

	void GpuTimer2::Release()
	{
		SAFE_RELEASE(m_queryHeap);
		SAFE_RELEASE(m_queryResourceCPU);
		SAFE_RELEASE(m_queryResourceGPU);
	}

	void GpuTimer2::Start(ID3D12GraphicsCommandList * commandList, UINT timestampIndex)
	{
		if (timestampIndex >= NUM_FRAMES)
			return;
		m_active = true;
		commandList->EndQuery(m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, timestampIndex * 2);
	}

	void GpuTimer2::Stop(ID3D12GraphicsCommandList * commandList, UINT timestampIndex)
	{
		if (timestampIndex >= NUM_FRAMES)
			return;
		m_active = false;
		commandList->EndQuery(m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, timestampIndex * 2 + 1);
	}

	void GpuTimer2::SetCommandQueue(ID3D12CommandQueue * commandQueue)
	{
		m_pCommandQueue = commandQueue;
	}

	UINT64 GpuTimer2::GetFreq()
	{
		UINT64 frq = 1;

		if (m_pCommandQueue)
			m_pCommandQueue->GetTimestampFrequency(&frq);

		return frq;

	}

	void GpuTimer2::ResolveQueryToCPU(ID3D12GraphicsCommandList * commandList, UINT timestampIndex)
	{
		if (timestampIndex >= NUM_FRAMES)
			return;
		commandList->ResolveQueryData(
			m_queryHeap,
			D3D12_QUERY_TYPE_TIMESTAMP,
			timestampIndex * 2,
			2,
			m_queryResourceCPU,
			sizeof(Timestamp) * timestampIndex
		);
	}

	void GpuTimer2::ResolveQueryToCPU(ID3D12GraphicsCommandList * commandList, UINT timestampIndexFirst, UINT timestampIndexLast)
	{
		UINT numToResolve = timestampIndexLast - timestampIndexFirst;
		commandList->ResolveQueryData(
			m_queryHeap,
			D3D12_QUERY_TYPE_TIMESTAMP,
			timestampIndexFirst * 2,
			numToResolve * 2,
			m_queryResourceCPU,
			sizeof(Timestamp) * timestampIndexFirst
		);
	}

	void GpuTimer2::ResolveQueryToGPU(ID3D12GraphicsCommandList * commandList, ID3D12Resource ** ppQueryResourceGPUOut)
	{
		_setGPUResourceState(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

		commandList->ResolveQueryData(m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, m_timerCount * 2, m_queryResourceGPU, 0);

		_setGPUResourceState(commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);

		if (ppQueryResourceGPUOut)
		{
			*ppQueryResourceGPUOut = m_queryResourceGPU;
		}
	}

	Timestamp GpuTimer2::GetTimestampPair(UINT timestampIndex)
	{
		Timestamp p{};
		
		Timestamp* mapMem = nullptr;
		D3D12_RANGE readRange{ sizeof(p) * timestampIndex, sizeof(p) * (timestampIndex + 1) };
		D3D12_RANGE writeRange{ 0, 0 };
		if (SUCCEEDED(m_queryResourceCPU->Map(0, &readRange, (void**)&mapMem)))
		{
			mapMem += timestampIndex;
			p = *mapMem;
			m_queryResourceCPU->Unmap(0, &writeRange);
		}
		
		return p;
	}

	void GpuTimer2::CalculateTime()
	{
		// Copy to CPU.
		UINT64 timeStamps[2];
		{
			void* mappedResource;
			D3D12_RANGE readRange{ 0, sizeof(UINT64) * m_timerCount * 2 };
			D3D12_RANGE writeRange{ 0, 0 };
			if (SUCCEEDED(m_queryResourceCPU->Map(0, &readRange, &mappedResource)))
			{
				memcpy(&timeStamps, mappedResource, sizeof(UINT64) * m_timerCount * 2);
				m_queryResourceCPU->Unmap(0, &writeRange);
			}
		}

		m_beginTime = timeStamps[0];
		m_endTime = timeStamps[1];

		m_deltaTime = m_endTime - m_beginTime;
	}

	UINT64 GpuTimer2::GetDeltaTime()
	{
		return m_deltaTime;
	}

	UINT64 GpuTimer2::GetEndTime()
	{
		return m_endTime;
	}

	UINT64 GpuTimer2::GetBeginTime()
	{
		return m_beginTime;
	}

	bool GpuTimer2::IsActive()
	{
		return m_active;
	}

	void GpuTimer2::_setGPUResourceState(ID3D12GraphicsCommandList * commandList, D3D12_RESOURCE_STATES prevState, D3D12_RESOURCE_STATES newState)
	{
		D3D12_RESOURCE_BARRIER barrierDesc = {};
		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Transition.pResource = m_queryResourceGPU;
		barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierDesc.Transition.StateBefore = prevState;
		barrierDesc.Transition.StateAfter = newState;

		commandList->ResourceBarrier(1, &barrierDesc);
	}
}
