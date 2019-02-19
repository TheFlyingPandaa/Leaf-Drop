#pragma once

class TextureAtlas : public LeafObject
{
private:
	TextureAtlas();
	~TextureAtlas();

public:
	static TextureAtlas * GetInstance();

	HRESULT Init(const std::wstring & name, const UINT & width, const UINT & height, const UINT & arraySize = 1, const UINT & maxMips = 1, const DXGI_FORMAT & format = DXGI_FORMAT_R8G8B8A8_UNORM);
	
	void Begin(ID3D12GraphicsCommandList * commandList) const;
	void End(ID3D12GraphicsCommandList * commandList) const;
	void CopySubresource(ID3D12Resource * resource, UINT & dstIndex, ID3D12GraphicsCommandList * commandList) const;
	void CopySubresource(ID3D12Resource *const* resource, const UINT & resourceSize, const UINT & dstIndex, ID3D12GraphicsCommandList * commandList) const;

	void SetGraphicsRootDescriptorTable(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList) const;
	void SetMagnusRootDescriptorTable(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList) const;

	ID3D12Resource * GetResource() const;

	void Release() override;
private:
	ID3D12Resource * m_textureAtlas = nullptr;
	UINT m_width = 0;
	UINT m_height = 0;
	UINT m_arraySize = 0;

	UINT m_descriptorHeapOffset = 0;

	CoreRender * m_coreRender = nullptr;
};

