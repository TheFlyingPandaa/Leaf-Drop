#pragma once
#include "Template/IRender.h"

class ComputePass : public IRender
{

public:
	ComputePass();
	~ComputePass();

	// Inherited via IRender
	virtual HRESULT Init() override;
	virtual void Update() override;
	virtual void Draw() override;
	virtual void Release() override;

private:
	HRESULT _Init();
	HRESULT _InitShaders();
	HRESULT _InitRootSignature();
	HRESULT _InitPipelineState();

	HRESULT _ExecuteCommandList();
private:

	D3D12_SHADER_BYTECODE m_computeShader{};

	ID3D12PipelineState * m_pipelineState = nullptr;
	ID3D12RootSignature * m_rootSignature = nullptr;

	ID3D12CommandQueue * m_commandQueue = nullptr;
};
