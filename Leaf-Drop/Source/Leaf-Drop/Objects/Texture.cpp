#include "CorePCH.h"
#include "Texture.h"
#include <ResourceUploadBatch.h>
#include "WICTextureLoader.h"

#include <wincodec.h>
#include <ScreenGrab.h>

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

	m_cpuHandle = { coreRender->GetCPUDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr + m_descriptorHeapOffset };

	device->CreateShaderResourceView(
		m_texture,
		&srvDesc,
		m_cpuHandle);

	coreRender->IterateResourceIndex(1);


	return hr;
}

void Texture::Map(UINT rootParameterIndex, ID3D12GraphicsCommandList * commandList) const
{
	
	ID3D12DescriptorHeap * heaps[] = { CoreRender::GetInstance()->GetCPUDescriptorHeap() };
	commandList->SetDescriptorHeaps(1, heaps);
	commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, 
		{ CoreRender::GetInstance()->GetCPUDescriptorHeap()->GetGPUDescriptorHandleForHeapStart().ptr + m_descriptorHeapOffset });
}

void Texture::Release()
{
	SAFE_RELEASE(m_texture);
}

ID3D12Resource* Texture::GetResource() const
{
	return this->m_texture;
}

HRESULT Texture::SaveToBMP(ID3D12CommandQueue * cq, ID3D12Resource * buffer, int index)
{
	using namespace DirectX;
	HRESULT hr = 0;

	std::wstring pathName = L"ScreenCapture/BMP/Frame_" + std::to_wstring(index) + L".bmp";

	hr = SaveWICTextureToFile(cq, buffer, GUID_ContainerFormatBmp, pathName.c_str(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_PRESENT);

	return hr;
}

HRESULT Texture::SaveToJPEG(ID3D12CommandQueue * cq, ID3D12Resource * buffer, int index)
{
	using namespace DirectX;
	HRESULT hr = 0;

	std::wstring pathName = L"ScreenCapture/JPEG/Frame_" + std::to_wstring(index) + L".jpg";

	hr = SaveWICTextureToFile(cq, buffer, GUID_ContainerFormatJpeg, pathName.c_str(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_PRESENT);

	return hr;
}

const D3D12_CPU_DESCRIPTOR_HANDLE & Texture::GetCpuHandle() const
{
	return m_cpuHandle;
}
