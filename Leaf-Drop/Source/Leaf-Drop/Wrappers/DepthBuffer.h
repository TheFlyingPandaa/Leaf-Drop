#pragma once

class DepthBuffer : public LeafObject
{
public:
	DepthBuffer();
	~DepthBuffer();

	HRESULT Init(const std::wstring & name, const UINT & width = 0, const UINT & height = 0, const UINT & arraySize = 1, const BOOL & asTexture = FALSE, const DXGI_FORMAT & format = DXGI_FORMAT_D32_FLOAT);
	void Clear(ID3D12GraphicsCommandList * commandList);
	void Bind(UINT rootParameterIndex, ID3D12GraphicsCommandList * commandList);

	const D3D12_CPU_DESCRIPTOR_HANDLE GetHandle() const;
	const UINT & GetArraySize() const;

	void SwapToDSV(ID3D12GraphicsCommandList * commandList = nullptr);
	void SwapToSRV(ID3D12GraphicsCommandList * commandList = nullptr);

	void Release() override;

private:
	UINT m_width = 0;
	UINT m_height = 0;
	UINT m_arraySize = 1;
	SIZE_T m_offset = 0;
	SIZE_T m_incrementalSize = 0;


	ID3D12Resource * m_depthBuffer[FRAME_BUFFER_COUNT] = { nullptr };
	ID3D12DescriptorHeap * m_descriptorHeap = nullptr;

};