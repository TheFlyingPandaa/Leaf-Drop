#include "CorePCH.h"
#include "ShaderResource.h"

ShaderResource::ShaderResource()
{
}

ShaderResource::~ShaderResource()
{
}

HRESULT ShaderResource::Init(const UINT & width, const UINT & height, const UINT & arraySize, const DXGI_FORMAT & format)
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
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	

	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		format,
		m_width, m_height,
		arraySize, 1, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	//resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

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
		
			m_descriptorHeapOffset = cr->GetCurrentResourceIndex() * cr->GetResourceDescriptorHeapSize();
			const D3D12_CPU_DESCRIPTOR_HANDLE handle =
			{ cr->GetResourceDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_descriptorHeapOffset };

			cr->GetDevice()->CreateShaderResourceView(
				m_resource[i],
				&srvDesc,
				handle);

			cr->IterateResourceIndex();
		}
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

	commandList->SetComputeRootShaderResourceView(rootParameterIndex, m_resource[currentFrame]->GetGPUVirtualAddress() + offset);
}

void ShaderResource::Clear(ID3D12GraphicsCommandList * commandList)
{
	/*static CoreRender * cr = CoreRender::GetInstance();
	static Window * wnd = Window::GetInstance();
	POINT wndSize = wnd->GetWindowSize();

	const UINT frameIndex = cr->GetFrameIndex();
	UINT8 * data = nullptr;
	D3D12_RANGE range{ 0,0 };

	if (SUCCEEDED(m_resource[frameIndex]->Map(0, &range, reinterpret_cast<void**>(&data))))
	{
		DXGI_FORMAT format = m_resource[frameIndex]->GetDesc().Format;
		
		ZeroMemory(data, m_width * m_height * _GetDXGIFormatBitsPerPixel(format));
		m_resource[frameIndex]->Unmap(0, &range);
	}
	*/
}

ID3D12Resource * ShaderResource::GetResource() const
{
	return m_resource[CoreRender::GetInstance()->GetFrameIndex()];
}

void ShaderResource::Release()
{
	for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
		SAFE_RELEASE(m_resource[i]);
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
