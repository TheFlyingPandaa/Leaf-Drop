#pragma once
#include "Template/IRender.h"
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

private:
	HRESULT _init();

};

