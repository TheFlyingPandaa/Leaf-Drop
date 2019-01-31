#include "CorePCH.h"
#include "DepthBuffer.h"

DepthBuffer::DepthBuffer()
{
}

DepthBuffer::~DepthBuffer()
{
}

HRESULT DepthBuffer::Init(const std::wstring & name, const UINT & width, const UINT & height, const UINT & arraySize, const BOOL & asTexture, const DXGI_FORMAT & format)
{
	HRESULT hr = 0;

	m_width = width;
	m_height = height;
	m_arraySize = arraySize;

	if (!width && !height)
	{
		UINT2 size =  Window::GetInstance()->GetWindowSize();
		m_width = size.x;
		m_height = size.y;
	}
	
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = arraySize * FRAME_BUFFER_COUNT;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	CoreRender * coreRender = CoreRender::GetInstance();

	if (SUCCEEDED(hr = coreRender->GetDevice()->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(&m_descriptorHeap))))
	{
		SET_NAME(m_descriptorHeap, std::wstring(name + L" DepthBuffer DescriptorHeap").c_str());
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = format;
		depthStencilDesc.ViewDimension = arraySize - 1 ? D3D12_DSV_DIMENSION_TEXTURE2DARRAY : D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
		if (D3D12_DSV_DIMENSION_TEXTURE2DARRAY == depthStencilDesc.ViewDimension)
		{
			depthStencilDesc.Texture2DArray.ArraySize = arraySize;
			depthStencilDesc.Texture2DArray.FirstArraySlice = 0;
			depthStencilDesc.Texture2DArray.MipSlice = 0;
		}

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = format;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);;

		m_incrementalSize = coreRender->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		for (UINT i = 0; i < FRAME_BUFFER_COUNT && SUCCEEDED(hr); i++)
		{
			if (SUCCEEDED(hr = coreRender->GetDevice()->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Tex2D(
					format,
					m_width, m_height,
					arraySize, 1, 1, 0,
					D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&depthOptimizedClearValue,
				IID_PPV_ARGS(&m_depthBuffer[i]))))
			{
				if (asTexture)
				{

					D3D12_DESCRIPTOR_HEAP_DESC textureHeapDesc = {};
					textureHeapDesc.NumDescriptors = 1;
					textureHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
					textureHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;


					D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
					srvDesc.ViewDimension = arraySize - 1 ? D3D12_SRV_DIMENSION_TEXTURE2DARRAY : D3D12_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					if (srvDesc.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2D)
					{
						srvDesc.Texture2D.MipLevels = 1;
					}
					else
					{
						srvDesc.Texture2DArray.ArraySize = arraySize;
						srvDesc.Texture2DArray.FirstArraySlice = 0;
						srvDesc.Texture2DArray.MipLevels = 1;
						srvDesc.Texture2DArray.MostDetailedMip = 0;
					}

					m_offset = coreRender->GetCurrentResourceIndex() * coreRender->GetResourceDescriptorHeapSize();

					const D3D12_CPU_DESCRIPTOR_HANDLE handle = { coreRender->GetResourceDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_offset };


					coreRender->GetDevice()->CreateShaderResourceView(
						m_depthBuffer[i],
						&srvDesc,
						handle);

				}

				SET_NAME(m_depthBuffer[i], std::wstring(name + L" DepthBuffer").c_str());
				coreRender->GetDevice()->CreateDepthStencilView(
					m_depthBuffer[i],
					&depthStencilDesc,
					{	m_descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + m_incrementalSize * i	});
			}
		}
	}

	return hr;
}

void DepthBuffer::Clear(ID3D12GraphicsCommandList * commandList)
{
	commandList->ClearDepthStencilView({ m_descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + CoreRender::GetInstance()->GetFrameIndex() * m_incrementalSize }, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void DepthBuffer::Bind(UINT rootParameterIndex, ID3D12GraphicsCommandList * commandList)
{

}

const D3D12_CPU_DESCRIPTOR_HANDLE DepthBuffer::GetHandle() const
{
	return { m_descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + CoreRender::GetInstance()->GetFrameIndex() * m_incrementalSize };
}

const UINT & DepthBuffer::GetArraySize() const
{
	return m_arraySize;
}

void DepthBuffer::SwapToDSV(ID3D12GraphicsCommandList * commandList)
{

}

void DepthBuffer::SwapToSRV(ID3D12GraphicsCommandList * commandList)
{

}

void DepthBuffer::Release()
{
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_depthBuffer[i]);
		
	}
	SAFE_RELEASE(m_descriptorHeap);

}
