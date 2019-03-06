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
		SAFE_RELEASE(m_GPUResource[i]);
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
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&m_GPUResource[i]))))
			return hr;

		SET_NAME(m_GPUResource[i], std::wstring(name + L" CPU_BUFFER_GPU_RESOURCE").c_str());

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

ID3D12Resource * CPUBuffer::GetResource() const
{
	return m_GPUResource[m_coreRender->GetFrameIndex()];
}

HRESULT CPUBuffer::BeginCopy(ID3D12GraphicsCommandList * commandList)
{
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_GPUResource[m_coreRender->GetFrameIndex()], D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	CD3DX12_RANGE range(0, 0);

	return m_uploadResource[m_coreRender->GetFrameIndex()]->Map(0, &range, reinterpret_cast<void**>(&m_address));
}

void CPUBuffer::EndCopy(ID3D12GraphicsCommandList * commandList)
{
	commandList->CopyResource(m_GPUResource[m_coreRender->GetFrameIndex()], m_uploadResource[m_coreRender->GetFrameIndex()]);

	CD3DX12_RANGE range(0, 0);
	m_uploadResource[m_coreRender->GetFrameIndex()]->Unmap(0, &range);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_GPUResource[m_coreRender->GetFrameIndex()], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));

}

void CPUBuffer::SetData(ID3D12GraphicsCommandList * commandList, void * data, const UINT & size, const UINT & offset)
{	
	memcpy(m_address + offset, data, size);
}

