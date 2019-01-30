#pragma once

class RenderTarget
{
public:
	RenderTarget();
	~RenderTarget();

	HRESULT Init(
		const std::wstring & name,
		const UINT & width = 0, 
		const UINT & height = 0, 
		const UINT arraySize = 1, 
		const DXGI_FORMAT & format = DXGI_FORMAT_R8G8B8A8_UNORM, 
		const BOOL & createSRV = FALSE);

private:
	ID3D12Resource * m_renderTarget[FRAME_BUFFER_COUNT] = { nullptr };
	ID3D12DescriptorHeap * m_renderTargetDescriptorHeap = nullptr;

	UINT m_width = 0;
	UINT m_height = 0;
	UINT m_arraySize = 0;

	UINT m_renderTargetDescriptorHeapSize = 0;

	Window * m_window = nullptr;
	CoreRender * m_coreRender = nullptr;
};

