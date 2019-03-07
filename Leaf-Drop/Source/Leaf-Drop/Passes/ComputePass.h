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

	void _SetLightData();

private:
	struct RAY_BOX
	{
		DirectX::XMFLOAT4A	viewerPos;
		DirectX::XMUINT4	info;
	};


	D3D12_SHADER_BYTECODE m_computeShader{};
	D3D12_SHADER_BYTECODE m_clearComputeShader{};

	ID3D12PipelineState * m_pipelineState = nullptr;
	
	ID3D12RootSignature * m_rootSignature = nullptr;
	//ID3D12RootSignature * m_clearRootSignature = nullptr;

	ID3D12CommandQueue *  m_commandQueue = nullptr;

	UINT8 m_computeIndex = 0;
	UAV * m_rayStencil = nullptr;

	ConstantBuffer m_meshData;
	ConstantBuffer m_ocTreeBuffer;

	UINT m_geometryRenderTargetsSize = 0;
	RenderTarget *const* m_geometryRenderTargets = nullptr;

	Fence m_fence;
	Fence m_fence2;

	OcTree m_staticOcTree;
	OcTree m_dynamicOcTree;

	ConstantBuffer m_squareIndex;
	ShaderResource m_rayTexture;

	UAV m_lightUav;
	ConstantBuffer m_lightsBuffer;

	std::vector<STRUCTS::ObjectValues> m_staticOctreeValues;
	std::vector<STRUCTS::ObjectValues> m_dynamicOctreeValues;

	GpuTimer timer;
};
