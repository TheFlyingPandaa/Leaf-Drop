#pragma once
#include "Template/IRender.h"
#include "../Wrappers/ShaderResource.h"
#include "../Wrappers/Fence.h"
#include "../Objects/OcTree.h"

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

	void Clear();

	void SetGeometryData(RenderTarget *const* renderTargets, const UINT & size);
	void SetRayData(UAV * rayStencil);

private:
	HRESULT _Init();
	HRESULT _InitShaders();

	HRESULT _InitRootSignature();

	HRESULT _InitPipelineState();

	HRESULT _ExecuteCommandList();

private:
	struct RAY_BOX
	{
		DirectX::XMFLOAT4A	viewerPos;
		DirectX::XMUINT4	info;
	};


	D3D12_SHADER_BYTECODE m_computeShader{};
	
	ID3D12PipelineState * m_pipelineState = nullptr;
	
	ID3D12RootSignature * m_rootSignature = nullptr;

	ID3D12CommandQueue *  m_commandQueue = nullptr;

	UAV * m_rayStencil = nullptr;

	UINT m_geometryRenderTargetsSize = 0;
	RenderTarget *const* m_geometryRenderTargets = nullptr;

	ConstantBuffer m_squareIndex;
	ConstantBuffer m_offsetBuffer;
	ShaderResource m_rayTexture;

	GpuTimer timer;
};
