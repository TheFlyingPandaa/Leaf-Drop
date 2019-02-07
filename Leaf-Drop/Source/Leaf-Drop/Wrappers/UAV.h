#pragma once
class UAV
{
public:
	UAV();
	~UAV();

	HRESULT Init(const std::wstring & name, const UINT & bufferSize, const UINT & maxElements, const UINT & elementSize);

	void Release();

	void Clear(ID3D12GraphicsCommandList * commandList);
	void Map(const UINT & rootParamtererIndex, ID3D12GraphicsCommandList * commandList);
	void Read(void * data, UINT * size);

private:
	ID3D12Resource * m_resource[FRAME_BUFFER_COUNT] = { nullptr };

	CoreRender * m_coreRender = nullptr;

	UINT m_bufferSize = 0;

	SIZE_T m_resourceDescriptorHeapOffset = 0;
};

