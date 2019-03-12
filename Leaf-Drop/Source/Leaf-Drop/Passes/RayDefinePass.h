#pragma once
#include "Template/IRender.h"

#define SAFE_RELEASE(p) {if ((p)) {(p)->Release(); (p) = nullptr;}}
class RayDefinePass : public IRender
{
public:
	RayDefinePass();
	~RayDefinePass();

	// Inherited via IRender
	virtual HRESULT Init() override;
	virtual void Update() override;
	virtual void Draw() override;
	virtual void Release() override;

	const UAV * GetRayStencil() const;
	void SetGeometryRenderTargets(RenderTarget *const* geometryRenderTargets, UINT arraySize);

private:
	struct ScreenQuad
	{
		struct Vertex
		{
			Vertex(float x, float y, float z, float u, float v)
			{
				pos = { x, y, z, 1.0f };
				uv = { u, v };
			}
			DirectX::XMFLOAT4 pos;
			DirectX::XMFLOAT2 uv;
		};

		UINT vertexBufferSize = 0;

		ID3D12Resource *	vertexBuffer = nullptr;
		ID3D12Resource *	 vertexUploadBuffer = nullptr;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

		std::vector<Vertex> mesh;

		inline void Release()
		{
			SAFE_RELEASE(vertexBuffer);
			SAFE_RELEASE(vertexUploadBuffer);	
		}
	};

	ID3D12PipelineState * m_pipelineState = nullptr;
	ID3D12RootSignature * m_rootSignature = nullptr;

	D3D12_INPUT_LAYOUT_DESC  m_inputLayoutDesc;

	D3D12_SHADER_BYTECODE m_vertexShader{};
	D3D12_SHADER_BYTECODE m_pixelShader{};

	D3D12_VIEWPORT m_viewport{};
	D3D12_RECT m_scissorRect{};

	ScreenQuad m_screenQuad;

	UINT m_geometryRenderTargetsSize = 0;
	RenderTarget *const* m_geometryRenderTargets = nullptr;


	UAV m_rayStencil;

	Fence m_fence;

private:
	HRESULT _Init();
	HRESULT _CreateScreenQuad(CoreRender * coreRender, const UINT & frameIndex, ID3D12GraphicsCommandList * commandList);
	HRESULT _InitRootSignature();
	HRESULT _InitShader();
	HRESULT _InitPipelineState();
	void	_CreateViewPort();
};

