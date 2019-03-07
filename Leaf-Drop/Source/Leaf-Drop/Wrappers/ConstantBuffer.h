#pragma once
#include "Template/LeafObject.h"

class CPUBuffer;

class ConstantBuffer : public LeafObject
{
public:
	enum CBV_TYPE
	{
		CONSTANT_BUFFER,
		STRUCTURED_BUFFER,
		BINDLESS_BUFFER
	};

public:
	ConstantBuffer();
	~ConstantBuffer();

	HRESULT Init(UINT initialSize, const std::wstring & name, const CBV_TYPE & type = CONSTANT_BUFFER, UINT sizeOfElement = 0);
	void Bind(UINT rootParameterIndex, ID3D12GraphicsCommandList * commandList, UINT offset = 0) const;
	void BindComputeShader(UINT rootParameterIndex, ID3D12GraphicsCommandList * commandList, UINT offset = 0) const;
	
	void SetData(void * data, UINT size, UINT offset = 0, const BOOL & forceAllBuffers = false);

	void Release() override;

	const D3D12_CPU_DESCRIPTOR_HANDLE GetHandle () const;

	ID3D12Resource * GetResource() const;

private:
	ID3D12Resource * m_resource[FRAME_BUFFER_COUNT] = { nullptr };
	//void * m_data = nullptr;
	UINT m_size = NULL;
	SIZE_T m_descriptorHeapOffset[FRAME_BUFFER_COUNT] = { 0 };
	CBV_TYPE m_type;

	UINT8 * m_resource_GPU_Location[FRAME_BUFFER_COUNT] = { nullptr };

private:
	CoreRender * m_coreRender = nullptr;

};
