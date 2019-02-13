#include "CorePCH.h"
#include "TextureAtlas.h"


TextureAtlas::TextureAtlas()
{
	m_coreRender = CoreRender::GetInstance();

}


TextureAtlas::~TextureAtlas()
{
}

HRESULT TextureAtlas::Init(const UINT& width, const UINT& height, const UINT& arraySize, const UINT & maxMips, const DXGI_FORMAT& format)
{
	HRESULT hr = 0;

	m_width = width;
	m_height = height;
	m_arraySize = arraySize;



	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		format,
		m_width, m_height,
		arraySize, maxMips, 1, 0,
		D3D12_RESOURCE_FLAG_NONE);

	if (SUCCEEDED(hr = m_coreRender->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&m_textureAtlas))))
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
			srvDesc.Texture2DArray.MipLevels = maxMips;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
		}

		m_descriptorHeapOffset = m_coreRender->GetCurrentResourceIndex() * m_coreRender->GetResourceDescriptorHeapSize();
		const D3D12_CPU_DESCRIPTOR_HANDLE handle =
		{ m_coreRender->GetResourceDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_descriptorHeapOffset };

		m_coreRender->GetDevice()->CreateShaderResourceView(
			m_textureAtlas,
			&srvDesc,
			handle);

		m_coreRender->IterateResourceIndex();

	}


	return hr;
}

void TextureAtlas::Begin(ID3D12GraphicsCommandList * commandList) const
{
	commandList->ResourceBarrier(1, 
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_textureAtlas, 
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			 D3D12_RESOURCE_STATE_COPY_DEST));
}

void TextureAtlas::End(ID3D12GraphicsCommandList * commandList) const
{
	commandList->ResourceBarrier(1, 
		&CD3DX12_RESOURCE_BARRIER::Transition(m_textureAtlas, 
			D3D12_RESOURCE_STATE_COPY_DEST, 
			 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

void TextureAtlas::CopySubresource(ID3D12Resource* resource, const UINT& dstIndex,
	ID3D12GraphicsCommandList* commandList) const
{
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));

	UINT counter = 0;
	const D3D12_RESOURCE_DESC desc = resource->GetDesc();
	for (UINT i = dstIndex; i < desc.DepthOrArraySize + dstIndex; i++)
	{
		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
		dstLocation.pResource = m_textureAtlas;
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = i;

		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		srcLocation.pResource = resource;
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		srcLocation.SubresourceIndex = counter++;

		D3D12_BOX box{ 0,0, 0, desc.Width, desc.Height, desc.MipLevels };

		commandList->CopyTextureRegion(
			&dstLocation,
			0,
			0,
			0,
			&srcLocation,
			&box);
	}
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

void TextureAtlas::CopySubresource(ID3D12Resource* const* resource, const UINT& resourceSize, const UINT& dstIndex,
	ID3D12GraphicsCommandList* commandList) const
{

	UINT counter = 0;
	
	for (UINT i = dstIndex; i < resourceSize + dstIndex; i++)
	{
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));
		
		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
		dstLocation.pResource = m_textureAtlas;
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = i;

		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		srcLocation.pResource = resource[i];
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		srcLocation.SubresourceIndex = counter++;

		commandList->CopyTextureRegion(
			&dstLocation,
			0,
			0,
			0,
			&srcLocation,
			nullptr);

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource[i], D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}
}


void TextureAtlas::SetGraphicsRootDescriptorTable(const UINT& rootParameterIndex,
	ID3D12GraphicsCommandList* commandList) const
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_coreRender->GetResourceDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += m_descriptorHeapOffset;

	commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, handle);
}

ID3D12Resource* TextureAtlas::GetResource() const
{
	return this->m_textureAtlas;
}

void TextureAtlas::Release()
{
	SAFE_RELEASE(m_textureAtlas);
}
