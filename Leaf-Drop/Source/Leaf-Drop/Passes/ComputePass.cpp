#include "CorePCH.h"
#include "ComputePass.h"
#include "../Wrappers/ShaderCreator.h"
#include "../Objects/Camera.h"

#define RAY_SQUARE_INDEX 0
#define RAY_TEXTURE 1

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
	
	//m_rayTexture.Clear(p_commandList[p_coreRender->GetFrameIndex()]);
}
#include <fstream>
void ComputePass::Draw()
{
	OpenCommandList(m_pipelineState);
	INT * rayTiles = nullptr;
	if (FAILED(m_rayTiles->Read(rayTiles)))
		return;
	m_rayTiles->Unmap();

	const UINT frameIndex = p_coreRender->GetFrameIndex();

	p_coreRender->SetResourceDescriptorHeap(p_commandList[frameIndex]);
	p_commandList[frameIndex]->SetComputeRootSignature(m_rootSignature);


	POINT windowSize = p_window->GetWindowSize();
	POINT nrOfRayTiles;
	nrOfRayTiles.x = windowSize.x / 32;
	nrOfRayTiles.y = windowSize.y / 32;
	
	RAY_BOX data;
	data.viewerPos.x = (float)windowSize.x * 0.5f;
	data.viewerPos.y = (float)windowSize.y * 0.5f;
	data.viewerPos.z = -(data.viewerPos.x / tan(Camera::GetActiveCamera()->GetFOV()));
	data.viewerPos.w = 1.0f;
	data.index.z = windowSize.x;
	data.index.w = windowSize.y;

	m_rayTexture.BindComputeShader(RAY_TEXTURE, p_commandList[frameIndex]);

	for (LONG y = 0; y < nrOfRayTiles.y; y++)
	{
		for (LONG x = 0; x < nrOfRayTiles.x; x++)
		{
			LONG index = x + y * nrOfRayTiles.y;
			UINT assDick = rayTiles[index];
			if (assDick == 1)
			{
				data.index.x = x;
				data.index.y = y;

				/*DirectX::XMFLOAT4 viewerPos = data.viewerPos;
				DirectX::XMFLOAT4 pixelPos = {(float)x, (float)y, 0.0f, 1.0f};
				DirectX::XMFLOAT4 ray;
				DirectX::XMStoreFloat4(&ray, DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat4(&pixelPos), DirectX::XMLoadFloat4(&viewerPos))));*/

				m_squareIndex.SetData(&data, sizeof(data));
				m_squareIndex.BindComputeShader(RAY_SQUARE_INDEX, p_commandList[frameIndex]);
				p_commandList[frameIndex]->Dispatch(1, 1, 1);
			}
		}
	}



	_ExecuteCommandList();

	m_rayTiles = nullptr;
}

void ComputePass::Release()
{
	SAFE_RELEASE(m_pipelineState);
	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_commandQueue);
}

void ComputePass::ClearDraw()
{
	const UINT frameIndex = p_coreRender->GetFrameIndex();
	OpenCommandList(m_clearPipelineState);

	p_coreRender->SetResourceDescriptorHeap(p_commandList[frameIndex]);
	p_commandList[frameIndex]->SetComputeRootSignature(m_rootSignature);

	POINT windowSize = p_window->GetWindowSize();
	RAY_BOX data;
	data.viewerPos.x = (float)windowSize.x * 0.5f;
	data.viewerPos.y = (float)windowSize.y * 0.5f;
	data.viewerPos.z = -(data.viewerPos.x / tan(Camera::GetActiveCamera()->GetFOV()));
	data.viewerPos.w = 1.0f;
	data.index.x = 0;
	data.index.y = 0;
	data.index.z = windowSize.x;
	data.index.w = windowSize.y;

	m_squareIndex.SetData(&data, sizeof(data));
	m_squareIndex.BindComputeShader(RAY_SQUARE_INDEX, p_commandList[frameIndex]);
	m_rayTexture.BindComputeShader(RAY_TEXTURE, p_commandList[frameIndex]);

	p_commandList[frameIndex]->Dispatch(1, 1, 1);

	//p_commandList[frameIndex]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_rayTexture.GetResource()));

	_ExecuteCommandList();
}

void ComputePass::SetRayTiles(UAV * rayTiles)
{
	m_rayTiles = rayTiles;
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
	if (FAILED(hr = _CreateFenceAndFenceEvent()))
	{
		return hr;
	}

	if (FAILED(hr = m_squareIndex.Init(sizeof(RAY_BOX), L"RaySqueare", ConstantBuffer::CONSTANT_BUFFER, sizeof(RAY_BOX))))
	{
		return hr;
	}

	if (FAILED(hr = m_rayTexture.Init("COkc")))
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


	if (FAILED(hr = ShaderCreator::CreateShader(L"..\\Leaf-Drop\\Source\\Leaf-Drop\\Shaders\\ComputePass\\ClearComputeShader.hlsl", blob, "cs_5_1")))
	{
		return hr;
	}
	m_clearComputeShader.pShaderBytecode = blob->GetBufferPointer();
	m_clearComputeShader.BytecodeLength = blob->GetBufferSize();

	return hr;
}

HRESULT ComputePass::_InitRootSignature()
{
	HRESULT hr = 0;

	D3D12_DESCRIPTOR_RANGE1 descRange = CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	rootParameters[RAY_SQUARE_INDEX].InitAsConstantBufferView(0);
	//rootParameters[RAY_TEXTURE].InitAsShaderResourceView(0);
	rootParameters[RAY_TEXTURE].InitAsDescriptorTable(1,&descRange);
	//rootParameters[RAY_TEXTURE].InitAsUnorderedAccessView(0);


	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;

	rootSignatureDesc.Init_1_1(
		_countof(rootParameters),
		rootParameters,
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

	//if (FAILED(hr = _InitClearRootSignature()))
	//{
	//	return hr;
	//}

	return hr;
}

HRESULT ComputePass::_InitClearRootSignature()
{
	HRESULT hr = 0;
	//
	//CD3DX12_ROOT_PARAMETER1 rootParameters[1];
	//
	//rootParameters[0].InitAsUnorderedAccessView(0);
	//
	//
	//CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//
	//rootSignatureDesc.Init_1_1(
	//	_countof(rootParameters),
	//	rootParameters,
	//	0,
	//	nullptr,
	//	D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
	//	D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
	//	D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
	//	D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
	//	D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
	//);
	//
	//ID3DBlob * signature = nullptr;
	//if (SUCCEEDED(hr = D3D12SerializeRootSignature(&rootSignatureDesc.Desc_1_0,
	//	D3D_ROOT_SIGNATURE_VERSION_1,
	//	&signature,
	//	nullptr)))
	//{
	//	if (FAILED(hr = p_coreRender->GetDevice()->CreateRootSignature(
	//		0,
	//		signature->GetBufferPointer(),
	//		signature->GetBufferSize(),
	//		IID_PPV_ARGS(&m_clearRootSignature))))
	//	{
	//		SAFE_RELEASE(m_clearRootSignature);
	//	}
	//}
	//
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


	//Create pipeline state
	if (FAILED(hr = _InitClearPipelineState()))
	{
		return hr;
	}

	return hr;
}

HRESULT ComputePass::_InitClearPipelineState()
{
	HRESULT hr = 0;

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipeDesc = {};

	computePipeDesc.pRootSignature = m_rootSignature;
	computePipeDesc.CS = m_clearComputeShader;

	if (FAILED(hr = p_coreRender->GetDevice()->CreateComputePipelineState(
		&computePipeDesc,
		IID_PPV_ARGS(&m_clearPipelineState))))
	{
		return hr;
	}

	return hr;

}

HRESULT ComputePass::_CreateFenceAndFenceEvent()
{
	HRESULT hr = 0;

	ID3D12Device * device = CoreRender::GetInstance()->GetDevice();

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence[i]))))
		{
			break;
		}
		m_fenceValue[i] = 0;
	}

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (nullptr == m_fenceEvent)
		return E_FAIL;

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

		if (FAILED(hr = m_commandQueue->Signal(m_fence[frameIndex], m_fenceValue[frameIndex])))
		{
			return hr;
		}
	}
	return hr;
}
