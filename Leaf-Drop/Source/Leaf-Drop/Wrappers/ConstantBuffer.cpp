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

HRESULT ConstantBuffer::Init(UINT initialSize, const std::wstring & name)
{
	HRESULT hr = 0;

	UINT bufferSize = (sizeof(initialSize) + 255) & ~255;

	D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_CUSTOM);
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = m_coreRender->GetDevice()->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&m_resource[i]))))
		{
			return hr;
		}

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

		m_coreRender->IterateResourceIndex();
	}

	return hr;
}

void ConstantBuffer::Bind(UINT rootParameterIndex, ID3D12GraphicsCommandList * commandList, UINT offset)
{
	const UINT currentFrame = m_coreRender->GetFrameIndex();
	commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, m_resource[currentFrame]->GetGPUVirtualAddress());
}

void ConstantBuffer::SetData(void * data, UINT size)
{
	const UINT currentFrame = m_coreRender->GetFrameIndex();
	
	CD3DX12_RANGE range(0, 0);
	UINT8 * dataLocation = nullptr;

	if (SUCCEEDED(m_resource[currentFrame]->Map(0, &range, reinterpret_cast<void**>(&dataLocation))))
	{

		memcpy(dataLocation, data, size);

		m_resource[currentFrame]->Unmap(0, &range);
	}

}
