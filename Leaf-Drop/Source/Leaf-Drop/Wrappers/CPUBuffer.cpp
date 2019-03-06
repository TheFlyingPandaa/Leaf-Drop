#include "CorePCH.h"
#include "CPUBuffer.h"


CPUBuffer::CPUBuffer()
{
	m_coreRender = CoreRender::GetInstance();
}


CPUBuffer::~CPUBuffer()
{
}

void CPUBuffer::Release()
{
	FOR_FRAME
	{
		SAFE_RELEASE(m_uploadResource[i]);
	}
}

HRESULT CPUBuffer::Init(const std::wstring & name, const UINT & bufferSize)
{
	HRESULT hr = 0;
	
	UINT alingedBufferSize = AlignAs256(bufferSize);

	D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(alingedBufferSize);
	
	FOR_FRAME
	{
		if (FAILED(hr = m_coreRender->GetDevice()->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_uploadResource[i]))))
			return hr;

		SET_NAME(m_uploadResource[i], std::wstring(name + L" CPU_BUFFER_UPLOAD_RESOURCe").c_str());

	}


	return hr;
}



HRESULT CPUBuffer::BeginCopy(ID3D12GraphicsCommandList * commandList)
{
	CD3DX12_RANGE range(0, 0);

	return m_uploadResource[m_coreRender->GetFrameIndex()]->Map(0, &range, reinterpret_cast<void**>(&m_address));
}

void CPUBuffer::EndCopy(ID3D12GraphicsCommandList * commandList, ID3D12Resource * targetResource)
{

	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(targetResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));
	
	commandList->CopyResource(targetResource, m_uploadResource[m_coreRender->GetFrameIndex()]);

	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(targetResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));


	CD3DX12_RANGE range(0, 0);
	m_uploadResource[m_coreRender->GetFrameIndex()]->Unmap(0, &range);

}

void CPUBuffer::SetData(ID3D12GraphicsCommandList * commandList, void * data, const UINT & size, const UINT & offset)
{	
	memcpy(m_address + offset, data, size);
}

