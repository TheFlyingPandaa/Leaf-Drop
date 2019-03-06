#pragma once
#include "Template/LeafObject.h"

class CPUBuffer :  public LeafObject
{
public:
	CPUBuffer();
	~CPUBuffer();

	// Inherited via LeafObject
	virtual void Release() override;

	HRESULT Init(const std::wstring & name, const UINT & bufferSize);

	ID3D12Resource * GetResource() const;

	HRESULT BeginCopy(ID3D12GraphicsCommandList * commandList);
	void EndCopy(ID3D12GraphicsCommandList * commandList);
	void SetData(ID3D12GraphicsCommandList * commandList, void * data, const UINT & size, const UINT & offset = 0);

private:
	CoreRender * m_coreRender;

	ID3D12Resource * m_uploadResource[FRAME_BUFFER_COUNT] = { nullptr };
	ID3D12Resource * m_GPUResource[FRAME_BUFFER_COUNT] = { nullptr };
	
	UINT8 * m_address = nullptr;
};

