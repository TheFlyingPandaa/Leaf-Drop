#pragma once

class TextureAtlas : public LeafObject
{
private:
	TextureAtlas();
	~TextureAtlas();

public:
	static TextureAtlas * GetInstance();

	void CopyBindless(Texture * texture);
	void BindlessGraphicsSetGraphicsRootDescriptorTable(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList) const;
	void BindlessComputeSetGraphicsRootDescriptorTable(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList) const;
	void Reset();


	void Release() override;
private:


	CoreRender * m_coreRender = nullptr;

	UINT m_nrOfTextures = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE m_bindlessStartHandle;
};

