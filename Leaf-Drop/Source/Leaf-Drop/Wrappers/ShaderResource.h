#pragma once

class ShaderResource
{
public:
	ShaderResource();
	~ShaderResource();

	HRESULT Init(
		const UINT & width = 0,
		const UINT & height = 0,
		const UINT& arraySize = 1,
		const DXGI_FORMAT& format = DXGI_FORMAT_R8G8B8A8_UNORM);

	void Bind(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList, UINT offset = 0);
	void BindComputeShader(const UINT & rootParameterIndex, ID3D12GraphicsCommandList * commandList, UINT offset = 0);
	void Clear(ID3D12GraphicsCommandList * commandList);

	ID3D12Resource * GetResource() const;

	void Release();
private:
	UINT m_width = 0;
	UINT m_height = 0;
	UINT m_arraySize = 1;

	ID3D12Resource * m_resource[FRAME_BUFFER_COUNT] = { nullptr };

	SIZE_T m_descriptorHeapOffset = 0;
private:
	int _GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);

};