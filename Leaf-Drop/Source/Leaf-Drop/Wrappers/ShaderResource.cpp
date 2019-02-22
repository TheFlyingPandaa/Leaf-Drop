#include "CorePCH.h"
#include "ShaderResource.h"

ShaderResource::ShaderResource()
{
}

ShaderResource::~ShaderResource()
{
}

HRESULT ShaderResource::Init(const std::wstring & name, const UINT & width, const UINT & height, const UINT & arraySize, const DXGI_FORMAT & format)
{
	HRESULT hr = 0;

	Window * wnd = Window::GetInstance();
	POINT wndSize = wnd->GetWindowSize();
	CoreRender * cr = CoreRender::GetInstance();

	m_width = width;
	m_height = height;
	m_arraySize = arraySize;

	if (width == 0 || height == 0)
	{
		m_width = wndSize.x;
		m_height = wndSize.y;
	}


	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_CUSTOM);
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		format,
		m_width, m_height,
		arraySize, 1, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (SUCCEEDED(hr = cr->GetDevice()->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			nullptr,
			IID_PPV_ARGS(&m_resource[i]))))
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
		
			m_descriptorHeapOffset[i] = cr->GetCurrentResourceIndex() * cr->GetResourceDescriptorHeapSize();
			const D3D12_CPU_DESCRIPTOR_HANDLE handle =
			{ cr->GetResourceDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_descriptorHeapOffset[i] };

			cr->GetDevice()->CreateShaderResourceView(
				m_resource[i],
				&srvDesc,
				handle);

			SET_NAME(m_resource[i], std::wstring(name + std::wstring(L" shaderResource ") + std::to_wstring(i)).c_str());

			cr->IterateResourceIndex();
		}
	}
	if (SUCCEEDED(hr = cr->GetDevice()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		nullptr,
		IID_PPV_ARGS(&m_clearResource))))
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

		SIZE_T offset = cr->GetCurrentResourceIndex() * cr->GetResourceDescriptorHeapSize();
		const D3D12_CPU_DESCRIPTOR_HANDLE handle =
		{ cr->GetResourceDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + offset };

		cr->GetDevice()->CreateShaderResourceView(
			m_clearResource,
			&srvDesc,
			handle);

		SET_NAME(m_clearResource, std::wstring(name + std::wstring(L" shaderResource Clear Resource ")).c_str());

		//D3D12_PLACED_SUBRESOURCE_FOOTPRINT psf;
		//UINT64 size = 0;

		////CoreRender::GetInstance()->GetDevice()->GetCopyableFootprints(&m_clearResource->GetDesc(), 0, 1, 0, &psf, nullptr, nullptr, &size);

		////memcpy()


		cr->IterateResourceIndex();
	}

	return hr;
}

void ShaderResource::Bind(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList, UINT offset)
{
	static CoreRender * cr = CoreRender::GetInstance();
	const UINT currentFrame = cr->GetFrameIndex();
	
	commandList->SetGraphicsRootShaderResourceView(rootParameterIndex, m_resource[currentFrame]->GetGPUVirtualAddress() + offset);
}

void ShaderResource::BindComputeShader(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList, UINT offset)
{
	static CoreRender * cr = CoreRender::GetInstance();
	const UINT currentFrame = cr->GetFrameIndex();

	commandList->SetComputeRootDescriptorTable(rootParameterIndex, {cr->GetResourceDescriptorHeap()->GetGPUDescriptorHandleForHeapStart().ptr + m_descriptorHeapOffset[currentFrame]});
}

void ShaderResource::BindComputeShaderUAV(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList, UINT offset)
{
	static CoreRender * cr = CoreRender::GetInstance();
	const UINT currentFrame = cr->GetFrameIndex();

	commandList->SetComputeRootUnorderedAccessView(rootParameterIndex, m_resource[currentFrame]->GetGPUVirtualAddress() + offset);

}

void ShaderResource::Clear(ID3D12GraphicsCommandList * commandList)
{
	static CoreRender * cr = CoreRender::GetInstance();
	static Window * wnd = Window::GetInstance();
	POINT wndSize = wnd->GetWindowSize();

	const UINT frameIndex = cr->GetFrameIndex();

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_resource[frameIndex], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));
	commandList->CopyResource(m_resource[frameIndex], m_clearResource);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_resource[frameIndex], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

}

ID3D12Resource * ShaderResource::GetResource() const
{
	return m_resource[CoreRender::GetInstance()->GetFrameIndex()];
}

void ShaderResource::Release()
{
	for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
		SAFE_RELEASE(m_resource[i]);
	SAFE_RELEASE(m_clearResource);
}

int ShaderResource::_GetDXGIFormatBitsPerPixel(DXGI_FORMAT & dxgiFormat)
{
	if (dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT) return 128;
	else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) return 64;
	else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM) return 64;
	else if (dxgiFormat == DXGI_FORMAT_R8G8B8A8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B8G8R8X8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM) return 32;

	else if (dxgiFormat == DXGI_FORMAT_R10G10B10A2_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B5G5R5A1_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_B5G6R5_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R32_FLOAT) return 32;
	else if (dxgiFormat == DXGI_FORMAT_R16_FLOAT) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R16_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R8_UNORM) return 8;

	return 8;
}
