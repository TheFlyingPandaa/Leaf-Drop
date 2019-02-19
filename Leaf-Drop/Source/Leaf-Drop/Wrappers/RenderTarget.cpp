#include "CorePCH.h"
#include "RenderTarget.h"


RenderTarget::RenderTarget()
{
	m_window = Window::GetInstance();
	m_coreRender = CoreRender::GetInstance();
}


RenderTarget::~RenderTarget()
{
	Release();

}

HRESULT RenderTarget::Init(const std::wstring & name, const UINT & width, const UINT & height, const UINT arraySize, const DXGI_FORMAT & format, const BOOL & createSRV)
{
	HRESULT hr = 0;

	m_width = width;
	m_height = height;
	m_arraySize = arraySize;

	if (m_width == 0 || m_height == 0)
	{
		POINT wndSize = m_window->GetWindowSize();
		m_width = wndSize.x;
		m_height = wndSize.y;
	}

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Alignment = 0;
	resourceDesc.DepthOrArraySize = FRAME_BUFFER_COUNT * arraySize;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.MipLevels = 1;
	resourceDesc.Width = m_width;
	resourceDesc.Height = m_height;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	const D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = m_coreRender->GetDevice()->GetResourceAllocationInfo(0, 1, &resourceDesc);

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT * arraySize;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ID3D12Heap * heap = nullptr;

	if (SUCCEEDED(hr = m_coreRender->GetDevice()->CreateDescriptorHeap(
		&rtvHeapDesc,
		IID_PPV_ARGS(&m_renderTargetDescriptorHeap))))
	{
		SET_NAME(m_renderTargetDescriptorHeap, std::wstring(name + std::wstring(L" RenderTargetDescriptorHeap")).c_str());
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

		D3D12_HEAP_DESC heapDesc{};
		heapDesc.Alignment = allocationInfo.Alignment;
		heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
		heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
		heapDesc.Properties = heapProperties;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = format;
		depthOptimizedClearValue.Color[0] = CLEAR_COLOR[0];
		depthOptimizedClearValue.Color[1] = CLEAR_COLOR[1];
		depthOptimizedClearValue.Color[2] = CLEAR_COLOR[2];
		depthOptimizedClearValue.Color[3] = CLEAR_COLOR[3];

		if (SUCCEEDED(hr = m_coreRender->GetDevice()->CreateHeap(
			&heapDesc,
			IID_PPV_ARGS(&heap))))
		{
			m_renderTargetDescriptorHeapSize = m_coreRender->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			
			D3D12_RESOURCE_DESC resourceDesc{};
			resourceDesc.Format = format;
			resourceDesc.Width = m_width;
			resourceDesc.Height = m_height;
			resourceDesc.DepthOrArraySize = arraySize;
			resourceDesc.MipLevels = 1;
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;


			for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
			{
				if (SUCCEEDED(hr = m_coreRender->GetDevice()->CreatePlacedResource(
					heap,
					0,
					&resourceDesc,
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					&depthOptimizedClearValue,
					IID_PPV_ARGS(&m_renderTarget[i]))))
				{
					SET_NAME(m_renderTarget[i], std::wstring(name + std::wstring(L" RenderTargetResource") + std::to_wstring(i)).c_str());

					D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
					renderTargetViewDesc.Format = format;
					renderTargetViewDesc.ViewDimension = arraySize > 1 ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY : D3D12_RTV_DIMENSION_TEXTURE2D;
					if (renderTargetViewDesc.ViewDimension == D3D12_RTV_DIMENSION_TEXTURE2D)
					{
						renderTargetViewDesc.Texture2D.MipSlice = 0;
						renderTargetViewDesc.Texture2D.PlaneSlice = 0;
					}
					else
					{
						renderTargetViewDesc.Texture2DArray.ArraySize = arraySize;
						renderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;
						renderTargetViewDesc.Texture2DArray.MipSlice = 0;
						renderTargetViewDesc.Texture2DArray.PlaneSlice = 0;
					}

					m_coreRender->GetDevice()->CreateRenderTargetView(m_renderTarget[i], &renderTargetViewDesc, rtvHandle);
					rtvHandle.ptr += m_renderTargetDescriptorHeapSize;

					if (createSRV)
					{
						D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
						srvDesc.Format = format;
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

						m_textureDescriptorHeapOffset = m_coreRender->GetCurrentResourceIndex() * m_coreRender->GetResourceDescriptorHeapSize();
						const D3D12_CPU_DESCRIPTOR_HANDLE handle = 
						{ m_coreRender->GetResourceDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_textureDescriptorHeapOffset };

						m_coreRender->GetDevice()->CreateShaderResourceView(
							m_renderTarget[i],
							&srvDesc,
							handle
						);

						m_coreRender->IterateResourceIndex();
					}
				}
			}

		}
	}
	SAFE_RELEASE(heap);
	return hr;
}

ID3D12DescriptorHeap * RenderTarget::GetDescriptorHeap() const
{
	return this->m_renderTargetDescriptorHeap;
}

const UINT & RenderTarget::GetRenderTargetDescriptorHeapSize() const
{
	return m_renderTargetDescriptorHeapSize;
}

void RenderTarget::SetGraphicsRootDescriptorTable(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList)
{
	commandList->SetGraphicsRootDescriptorTable(rootParameterIndex,
		{ m_coreRender->GetResourceDescriptorHeap()->GetGPUDescriptorHandleForHeapStart().ptr + m_textureDescriptorHeapOffset });
}

ID3D12Resource * RenderTarget::GetResource()
{
	return m_renderTarget[CoreRender::GetInstance()->GetFrameIndex()];
}

void RenderTarget::SwitchToRTV(ID3D12GraphicsCommandList * commandList)
{	
	const UINT frameIndex = m_coreRender->GetFrameIndex();
	if (m_renderTargetCurrentResourceState[frameIndex] == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget[frameIndex], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_renderTargetCurrentResourceState[frameIndex] = D3D12_RESOURCE_STATE_RENDER_TARGET;
}

void RenderTarget::SwitchToSRV(ID3D12GraphicsCommandList * commandList)
{
	const UINT frameIndex = m_coreRender->GetFrameIndex();
	if (m_renderTargetCurrentResourceState[frameIndex] == D3D12_RESOURCE_STATE_RENDER_TARGET)
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget[m_coreRender->GetFrameIndex()], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	m_renderTargetCurrentResourceState[frameIndex] = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

}

void RenderTarget::Release()
{
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(m_renderTarget[i]);
	}
	SAFE_RELEASE(m_renderTargetDescriptorHeap);
}


