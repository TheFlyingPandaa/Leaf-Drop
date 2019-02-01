#include "CorePCH.h"
#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer()
{
	m_coreRender = CoreRender::GetInstance();
}

ConstantBuffer::~ConstantBuffer()
{
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_resource[i]);
	}
	m_data = nullptr;
}

HRESULT ConstantBuffer::Init(UINT initialSize, const std::wstring & name, const CBV_TYPE & type, UINT sizeOfElement)
{
	m_type = type;

	HRESULT hr = 0;

	UINT bufferSize = initialSize + 255 & ~255;
	
	//UINT bufferSize = initialSize;

	D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_CUSTOM);
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	resDesc.Flags = type ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

	D3D12_RESOURCE_STATES state = type ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : D3D12_RESOURCE_STATE_COMMON;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = m_coreRender->GetDevice()->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			state,
			nullptr,
			IID_PPV_ARGS(&m_resource[i]))))
		{
			return hr;
		}

		switch (type)
		{
		case STRUCTURED_BUFFER:
		{
			SET_NAME(m_resource[i], std::wstring(name + L" StructuredBuffer" + std::to_wstring(i)).c_str());
			D3D12_BUFFER_UAV uav{};
			uav.NumElements = 1;
			uav.FirstElement = 0;
			uav.StructureByteStride = bufferSize;
			uav.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

			D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc{};
			unorderedAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
			unorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			unorderedAccessViewDesc.Buffer = uav;

			m_descriptorHeapOffset = m_coreRender->GetCurrentResourceIndex() * m_coreRender->GetResourceDescriptorHeapSize();
			const D3D12_CPU_DESCRIPTOR_HANDLE handle =
			{ m_coreRender->GetResourceDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_descriptorHeapOffset };


			m_coreRender->GetDevice()->CreateUnorderedAccessView(
				m_resource[i],
				nullptr,
				&unorderedAccessViewDesc,
				handle);
		}
			break;
		case CONSTANT_BUFFER:
		{
			SET_NAME(m_resource[i], std::wstring(name + L" Constantbuffer" + std::to_wstring(i)).c_str());

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = m_resource[i]->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = bufferSize;
			m_descriptorHeapOffset = m_coreRender->GetCurrentResourceIndex() * m_coreRender->GetResourceDescriptorHeapSize();
			const D3D12_CPU_DESCRIPTOR_HANDLE handle =
			{ m_coreRender->GetResourceDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_descriptorHeapOffset };

			m_coreRender->GetDevice()->CreateConstantBufferView(
				&cbvDesc,
				handle);
		}
			break;
		}
		CD3DX12_RANGE range(0, 0);

		if (FAILED(hr = m_resource[i]->Map(0, &range, reinterpret_cast<void**>(&m_resource_GPU_Location[i]))))
		{

		}

		m_coreRender->IterateResourceIndex();
	}

	return hr;
}

void ConstantBuffer::Bind(UINT rootParameterIndex, ID3D12GraphicsCommandList * commandList, UINT offset)
{
	const UINT currentFrame = m_coreRender->GetFrameIndex();

	switch (m_type)
	{
	case ConstantBuffer::CONSTANT_BUFFER:
		commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, m_resource[currentFrame]->GetGPUVirtualAddress() + offset);
		break;
	case ConstantBuffer::STRUCTURED_BUFFER:
		commandList->SetGraphicsRootShaderResourceView(rootParameterIndex, m_resource[currentFrame]->GetGPUVirtualAddress() + offset);
		break;
	}
	
}

void ConstantBuffer::SetData(void * data, UINT size, UINT offset)
{
	const UINT currentFrame = m_coreRender->GetFrameIndex();
	memcpy(m_resource_GPU_Location[currentFrame] + offset, data, size);
}
