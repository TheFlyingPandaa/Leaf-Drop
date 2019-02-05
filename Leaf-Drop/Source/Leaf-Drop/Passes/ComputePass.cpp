#include "CorePCH.h"
#include "ComputePass.h"
#include "../Wrappers/ShaderCreator.h"
ComputePass::ComputePass()
{
}

ComputePass::~ComputePass()
{
}

HRESULT ComputePass::Init()
{
	HRESULT hr = 0;

	if (FAILED(hr = _Init()))
	{
		return hr;
	}

	return hr;
}

void ComputePass::Update()
{

}

void ComputePass::Draw()
{
	OpenCommandList(m_pipelineState);
	const UINT frameIndex = p_coreRender->GetFrameIndex();

	p_commandList[frameIndex]->Dispatch(100, 100, 1);

	_ExecuteCommandList();
}

void ComputePass::Release()
{
	SAFE_RELEASE(m_pipelineState);
	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_commandQueue);
}

HRESULT ComputePass::_Init()
{
	HRESULT hr = 0;

	//Compute command list
	D3D12_COMMAND_QUEUE_DESC desc{};
	desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;

	if (FAILED(hr = p_coreRender->GetDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue))))
	{
		return hr;
	}
	if (FAILED(hr = p_CreateCommandList(L"Compute", D3D12_COMMAND_LIST_TYPE_COMPUTE)))
	{
		return hr;
	}
	if (FAILED(hr = OpenCommandList()))
	{
		return hr;
	}
	if (FAILED(hr = _InitShaders()))
	{
		return hr;
	}
	if (FAILED(hr = _InitRootSignature()))
	{
		return hr;
	}
	if (FAILED(hr = _InitPipelineState()))
	{
		return hr;
	}

	if (FAILED(hr = _ExecuteCommandList()))
	{
		return hr;
	}

	return hr;
}

HRESULT ComputePass::_InitShaders()
{
	HRESULT hr = 0;
	ID3DBlob * blob = nullptr;
	if (FAILED(hr = ShaderCreator::CreateShader(L"..\\Leaf-Drop\\Source\\Leaf-Drop\\Shaders\\ComputePass\\DefaultComputeShader.hlsl", blob, "cs_5_1")))
	{
		return hr;
	}
	m_computeShader.pShaderBytecode = blob->GetBufferPointer();
	m_computeShader.BytecodeLength = blob->GetBufferSize();

	return hr;
}

HRESULT ComputePass::_InitRootSignature()
{
	HRESULT hr = 0;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;

	rootSignatureDesc.Init_1_1(
		NULL,
		nullptr,
		0,
		nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
	);

	ID3DBlob * signature = nullptr;
	if (SUCCEEDED(hr = D3D12SerializeRootSignature(&rootSignatureDesc.Desc_1_0,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signature,
		nullptr)))
	{
		if (FAILED(hr = p_coreRender->GetDevice()->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature))))
		{
			SAFE_RELEASE(m_rootSignature);
		}
	}

	return hr;
}

HRESULT ComputePass::_InitPipelineState()
{
	HRESULT hr = 0;

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipeDesc = {};

	computePipeDesc.pRootSignature = m_rootSignature;
	computePipeDesc.CS = m_computeShader;

	if (FAILED(hr = p_coreRender->GetDevice()->CreateComputePipelineState(
		&computePipeDesc,
		IID_PPV_ARGS(&m_pipelineState))))
	{
		return hr;
	}

	return hr;
}

HRESULT ComputePass::_ExecuteCommandList()
{
	HRESULT hr = 0;
	const UINT frameIndex = p_coreRender->GetFrameIndex();

	if (SUCCEEDED(hr = p_commandList[frameIndex]->Close()))
	{
		ID3D12CommandList* ppCommandLists[] = { p_commandList[frameIndex] };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}
	return hr;
}
