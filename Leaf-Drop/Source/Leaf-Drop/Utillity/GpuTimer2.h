#pragma once
// CopyRight Stefan Petersson 2019

namespace GPU
{
	struct Timestamp
	{
		
		UINT64 Start = 0;
		UINT64 Stop = 0;
	};
	
	class GpuTimer2
	{
	public:
		GpuTimer2();
		~GpuTimer2();

		HRESULT Init(UINT numberOfTimers, D3D12_QUERY_HEAP_TYPE heapType = D3D12_QUERY_HEAP_TYPE_TIMESTAMP);
		void Release();

		void Start(ID3D12GraphicsCommandList * commandList, UINT timestampIndex);
		void Stop(ID3D12GraphicsCommandList* commandList, UINT timestampIndex);

		void SetCommandQueue(ID3D12CommandQueue * commandQueue);
		ID3D12CommandQueue* GetCommandQueue() { return m_pCommandQueue; };
		UINT64 GetFreq();

		void ResolveQueryToCPU(ID3D12GraphicsCommandList* commandList, UINT timestampIndex);
		void ResolveQueryToCPU(ID3D12GraphicsCommandList* commandList, UINT timestampIndexFirst, UINT timestampIndexLast);
		void ResolveQueryToGPU(ID3D12GraphicsCommandList* commandList, ID3D12Resource** ppQueryResourceGPUOut);

		Timestamp GetTimestampPair(UINT timestampIndex);
		void CalculateTime();

		UINT64 GetDeltaTime();
		UINT64 GetEndTime();
		UINT64 GetBeginTime();

		bool IsActive();

	private:
		void _setGPUResourceState(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES prevState, D3D12_RESOURCE_STATES newState);

		ID3D12Device*		m_pDevice = nullptr;
		ID3D12QueryHeap*	m_queryHeap = nullptr;
		ID3D12Resource*		m_queryResourceCPU = nullptr;
		ID3D12Resource*		m_queryResourceGPU = nullptr;
		ID3D12CommandQueue*	m_pCommandQueue = nullptr;

		bool	m_active = false;
		UINT64	m_deltaTime = 0;
		UINT64	m_beginTime = 0;
		UINT64	m_endTime = 0;
		UINT	m_timerCount = 0;

	};
}


