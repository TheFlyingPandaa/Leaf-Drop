#pragma once
#include "Template/IRender.h"
#include "../Wrappers/RenderTarget.h"
#include "../Wrappers/ConstantBuffer.h"
#include "../Wrappers/DepthBuffer.h"

class GeometryPass : public IRender
{
private:
	static const UINT ROOT_PARAMETERS = 1;
public:
	GeometryPass();
	~GeometryPass();

	// Inherited via IRender
	HRESULT Init() override;
	void Update() override;
	void Draw() override;
	void Release() override;

private:
	ID3D12PipelineState * m_pipelineState = nullptr;
	ID3D12RootSignature * m_rootSignature = nullptr;

	D3D12_INPUT_LAYOUT_DESC  m_inputLayoutDesc;

	D3D12_SHADER_BYTECODE m_vertexShader{}; 
	D3D12_SHADER_BYTECODE m_pixelShader{};

	RenderTarget * m_renderTarget = nullptr;

	D3D12_VIEWPORT m_viewport{};
	D3D12_RECT m_scissorRect{};

	DepthBuffer m_depthBuffer;

	ConstantBuffer m_camBuffer;
	ConstantBuffer m_worldMatrices;

private:
	HRESULT _Init();
	HRESULT _InitRootSignature();
	HRESULT _InitShader();
	HRESULT _InitPipelineState();
	void _CreateViewPort();

};

