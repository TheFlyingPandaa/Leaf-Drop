#include "CorePCH.h"
#include "Texture.h"
#include <ResourceUploadBatch.h>
#include "WICTextureLoader.h"

Texture::Texture()
{
}


Texture::~Texture()
{
	Release();
}

HRESULT Texture::LoadTexture(const std::wstring & path)
{
	static bool init = false;
	HRESULT hr = 0;
	if (!init)
	{
		if (FAILED(hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED)))
			return hr;

		init = !init;
	}


	CoreRender * coreRender = CoreRender::GetInstance();
	ID3D12Device * device = coreRender->GetDevice();

	using namespace DirectX;

	ResourceUploadBatch resourceUpload(device);
	
	resourceUpload.Begin();

	if (FAILED(hr = CreateWICTextureFromFile(device, resourceUpload, path.c_str(),
		&m_texture, true)))
	{
		return hr;
	}

	// Upload the resources to the GPU.
	auto uploadResourcesFinished = resourceUpload.End(coreRender->GetCommandQueue());

	// Wait for the upload thread to terminate
	uploadResourcesFinished.wait();


	const D3D12_RESOURCE_DESC resourceDesc = m_texture->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = resourceDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;

	m_descriptorHeapOffset = coreRender->GetCurrentResourceIndex() * coreRender->GetResourceDescriptorHeapSize();

	D3D12_CPU_DESCRIPTOR_HANDLE handle = { coreRender->GetResourceDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_descriptorHeapOffset };

	device->CreateShaderResourceView(
		m_texture,
		&srvDesc,
		handle);

	coreRender->IterateResourceIndex();


	return hr;
}

void Texture::Map(UINT rootParameterIndex, ID3D12GraphicsCommandList * commandList)
{
	
	ID3D12DescriptorHeap * heaps[] = { CoreRender::GetInstance()->GetResourceDescriptorHeap() };
	commandList->SetDescriptorHeaps(1, heaps);
	commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, 
		{ CoreRender::GetInstance()->GetResourceDescriptorHeap()->GetGPUDescriptorHandleForHeapStart().ptr + m_descriptorHeapOffset });
}

void Texture::Release()
{
	SAFE_RELEASE(m_texture);
}
