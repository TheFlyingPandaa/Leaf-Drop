#pragma once
class UAV : public LeafObject
{
public:
	UAV();
	~UAV();

	HRESULT Init(const std::wstring & name, const UINT & bufferSize, const UINT & maxElements, const UINT & elementSize);


	void Clear(ID3D12GraphicsCommandList * commandList);
	void Bind(const UINT & rootParamtererIndex, ID3D12GraphicsCommandList * commandList);
	void BindCompute(const UINT & rootParamtererIndex, ID3D12GraphicsCommandList * commandList);
	void BindComputeSrv(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList);
	void SetGraphicsRootShaderResourceView(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList);
	template <typename T>
	HRESULT Read(T*& data);
	void Unmap();

	HRESULT ConstantMap();
	void UnmapAll();
	void CopyData(void * data, const UINT & size, const UINT & offset = 0); /*The Uav must be mapped for this to work*/
	

	ID3D12Resource *const* GetResource() const;

	UINT prevFrame;

	void Release() override;

private:
	UINT8 * m_gpuAddress[FRAME_BUFFER_COUNT] = { nullptr };
	ID3D12Resource * m_resource[FRAME_BUFFER_COUNT] = { nullptr };

	CoreRender * m_coreRender = nullptr;

	UINT m_bufferSize = 0;

	SIZE_T m_resourceDescriptorHeapOffset = 0;
};

template <typename T>
HRESULT UAV::Read(T *& data)
{
	HRESULT hr = 0;
	D3D12_RANGE range{ 0, m_bufferSize };
	hr = m_resource[prevFrame]->Map(0, &range, reinterpret_cast<void**>(&data));
	return hr;
}


