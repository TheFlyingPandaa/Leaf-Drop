#pragma once
#include <WICTextureLoader.h>

class Texture
{
public:
	Texture();
	~Texture();
	
	HRESULT LoadTexture(const std::wstring & path);
	void Map(UINT rootParameterIndex, ID3D12GraphicsCommandList * commandList) const;

	void Release();

	ID3D12Resource * GetResource() const;

	static HRESULT SaveToBMP(ID3D12CommandQueue * cq, ID3D12Resource * buffer, int index);
	static HRESULT SaveToJPEG(ID3D12CommandQueue * cq, ID3D12Resource * buffer, int index);

	const D3D12_CPU_DESCRIPTOR_HANDLE & GetCpuHandle() const;

private:
	ID3D12Resource * m_texture = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;

	SIZE_T m_descriptorHeapOffset = 0;
};

