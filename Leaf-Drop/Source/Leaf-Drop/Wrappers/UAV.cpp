#include "CorePCH.h"
#include "UAV.h"


UAV::UAV()
{
	m_coreRender = CoreRender::GetInstance();
}


UAV::~UAV()
{
	Release();
}

HRESULT UAV::Init(const std::wstring & name, const UINT & bufferSize, const UINT & maxElements, const UINT & elementSize)
{
	const UINT alignedBufferSize = bufferSize + 255 & ~255;
	m_bufferSize = alignedBufferSize;
	HRESULT hr = 0;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_CUSTOM);
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

		D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(alignedBufferSize);
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		
		if (FAILED(hr = m_coreRender->GetDevice()->CreateCommittedResource(&heapProp,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&resourceDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&m_resource[i]))))
		{
			return hr;
		}

		SET_NAME(m_resource[i], std::wstring(name + L" UAV" + std::to_wstring(i)).c_str());

		D3D12_BUFFER_UAV uav{};
		uav.NumElements = maxElements;
		uav.FirstElement = 0;
		uav.StructureByteStride = elementSize;
		uav.CounterOffsetInBytes = 0;
		uav.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc{};
		unorderedAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
		unorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		unorderedAccessViewDesc.Buffer = uav;

		m_resourceDescriptorHeapOffset = m_coreRender->GetCurrentResourceIndex() * m_coreRender->GetResourceDescriptorHeapSize();

		const D3D12_CPU_DESCRIPTOR_HANDLE handle = { m_coreRender->GetResourceDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_resourceDescriptorHeapOffset };

		m_coreRender->GetDevice()->CreateUnorderedAccessView(m_resource[i], nullptr, &unorderedAccessViewDesc, handle);

		m_coreRender->IterateResourceIndex();
	}
	return hr;
}

void UAV::Release()
{
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_resource[i]);
	}
}

void UAV::Clear(ID3D12GraphicsCommandList * commandList)
{
	const UINT frameIndex = m_coreRender->GetFrameIndex();
	UINT8 * data = nullptr;
	D3D12_RANGE range{ 0,0 };

	if (SUCCEEDED(m_resource[frameIndex]->Map(0, &range, reinterpret_cast<void**>(&data))))
	{
		ZeroMemory(data, m_bufferSize);
		m_resource[frameIndex]->Unmap(0,&range);
	}
}

void UAV::Map(const UINT & rootParamtererIndex, ID3D12GraphicsCommandList * commandList)
{
	const UINT frameIndex = m_coreRender->GetFrameIndex();
	commandList->SetGraphicsRootUnorderedAccessView(rootParamtererIndex, m_resource[frameIndex]->GetGPUVirtualAddress());
}

void UAV::SetGraphicsRootShaderResourceView(const UINT& rootParameterIndex, ID3D12GraphicsCommandList* commandList)
{
	const UINT frameIndex = m_coreRender->GetFrameIndex();
	commandList->SetGraphicsRootShaderResourceView(rootParameterIndex, m_resource[frameIndex]->GetGPUVirtualAddress());
}

void UAV::Unmap()
{
	D3D12_RANGE range{ 0, m_bufferSize };

	m_resource[prevFrame]->Unmap(0, &range);


	m_gpuAddress[prevFrame] = { nullptr };
	
}


HRESULT UAV::ConstantMap()
{
	HRESULT hr = 0;
	
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		D3D12_RANGE range{ 0,0 };
		if (SUCCEEDED(hr = m_resource[i]->Map(0, &range, reinterpret_cast<void**>(&m_gpuAddress[i]))))
		{
			
		}
	}
	return hr;
}

void UAV::UnmapAll()
{
	D3D12_RANGE range{ 0, m_bufferSize };

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		m_resource[i]->Unmap(0, &range);


		m_gpuAddress[i] = { nullptr };

	}

}

void UAV::CopyData(void* data, const UINT& size, const UINT& offset)
{
	memcpy(m_gpuAddress[m_coreRender->GetFrameIndex()] + offset, data, size);
}


ID3D12Resource * const * UAV::GetResource() const
{
	return m_resource;
}
