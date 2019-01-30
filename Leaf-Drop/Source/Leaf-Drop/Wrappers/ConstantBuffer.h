#pragma once

class ConstantBuffer
{
public:
	ConstantBuffer();
	~ConstantBuffer();

	HRESULT Init(UINT initialSize, const std::wstring & name);
	void Bind(UINT rootParameterIndex, ID3D12GraphicsCommandList * commandList, UINT offset = 0);
	void SetData(void * data, UINT size);

private:
	ID3D12Resource * m_resource[FRAME_BUFFER_COUNT] = { nullptr };
	void * m_data = nullptr;
	UINT m_size = NULL;
	SIZE_T m_descriptorHeapOffset = 0;


private:
	CoreRender * m_coreRender = nullptr;

};