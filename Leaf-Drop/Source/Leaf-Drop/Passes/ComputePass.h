#pragma once
#include "Template/IRender.h"
#include "../Wrappers/ShaderResource.h"
#include "../Wrappers/Fence.h"

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

	void ClearDraw();

	void SetGeometryData(RenderTarget *const* renderTargets, const UINT & size);
	void SetRayData(UAV * rayStencil, UAV * counterStencil);

private:
	HRESULT _Init();
	HRESULT _InitShaders();

	HRESULT _InitRootSignature();
	HRESULT _InitClearRootSignature();

	HRESULT _InitPipelineState();
	HRESULT _InitClearPipelineState();



	HRESULT _ExecuteCommandList();

private:
	struct RAY_BOX
	{
		DirectX::XMFLOAT4A	viewerPos;
		DirectX::XMUINT4	info;
	};


	D3D12_SHADER_BYTECODE m_computeShader{};
	D3D12_SHADER_BYTECODE m_clearComputeShader{};

	ID3D12PipelineState * m_pipelineState = nullptr;
	ID3D12PipelineState * m_clearPipelineState = nullptr;

	ID3D12RootSignature * m_rootSignature = nullptr;
	//ID3D12RootSignature * m_clearRootSignature = nullptr;

	ID3D12CommandQueue *  m_commandQueue = nullptr;

	UINT8 m_computeIndex = 0;
	UAV * m_rayStencil = nullptr;
	UAV * m_counterStencil = nullptr;

	ConstantBuffer m_meshTriangles;

	UINT m_geometryRenderTargetsSize = 0;
	RenderTarget *const* m_geometryRenderTargets = nullptr;

	Fence m_fence;

	ConstantBuffer m_squareIndex;
	ShaderResource m_rayTexture;
};
