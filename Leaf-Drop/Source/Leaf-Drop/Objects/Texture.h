#pragma once
#include <WICTextureLoader.h>

class Texture
{
public:
	Texture();
	~Texture();
	
	HRESULT LoadTexture(const std::wstring & path);
	void Map(UINT rootParameterIndex, ID3D12GraphicsCommandList * commandList);

	void Release();

	ID3D12Resource * GetResource() const;

private:
	ID3D12Resource * m_texture = nullptr;

	SIZE_T m_descriptorHeapOffset = 0;
};

