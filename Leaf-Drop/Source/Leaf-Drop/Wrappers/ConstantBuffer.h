#pragma once

class ConstantBuffer
{
public:
	enum CBV_TYPE
	{
		CONSTANT_BUFFER,
		STRUCTURED_BUFFER
	};

public:
	ConstantBuffer();
	~ConstantBuffer();

	HRESULT Init(UINT initialSize, const std::wstring & name, const CBV_TYPE & type = CONSTANT_BUFFER, UINT sizeOfElement = 0);
	void Bind(UINT rootParameterIndex, ID3D12GraphicsCommandList * commandList, UINT offset = 0);
	void SetData(void * data, UINT size, UINT offset = 0);

private:
	ID3D12Resource * m_resource[FRAME_BUFFER_COUNT] = { nullptr };
	void * m_data = nullptr;
	UINT m_size = NULL;
	SIZE_T m_descriptorHeapOffset = 0;
	CBV_TYPE m_type;

private:
	CoreRender * m_coreRender = nullptr;

};