#pragma once
#include "Template/IRender.h"


class PrePass : public IRender
{
private:
	static const UINT RENDER_TARGETS = 1;
public:
	PrePass();
	~PrePass();

	// Inherited via IRender
	virtual HRESULT Init() override;
	virtual void Update() override;
	virtual void Draw() override;
	virtual void Release() override;

private:
	ID3D12PipelineState * m_pipelineState = nullptr;
	ID3D12RootSignature * m_rootSignature = nullptr;

	ConstantBuffer m_camBuffer;
	ConstantBuffer m_worldMatrices;

	D3D12_INPUT_LAYOUT_DESC  m_inputLayoutDesc;

	D3D12_SHADER_BYTECODE m_vertexShader{};

	RenderTarget m_renderTarget;

	D3D12_VIEWPORT m_viewport{};
	D3D12_RECT m_scissorRect{};

	DepthBuffer m_depthBuffer;

private:
	HRESULT _Init();
	HRESULT _InitRootSignature();
	HRESULT _InitShader();
	HRESULT _InitPipelineState();
	void _CreateViewPort();
};

