#include "CorePCH.h"
#include "ComputePass.h"
#include "../Wrappers/ShaderCreator.h"
#include "../Objects/Camera.h"
#include "../Objects/StaticMesh.h"

#include <iostream>

#define RAY_SQUARE_INDEX 0
#define RAY_TEXTURE 1
#define RAY_INDICES 2
#define TRIANGLES	3

struct Vertex
{
	DirectX::XMFLOAT4 pos;
	DirectX::XMFLOAT4 normal;
	DirectX::XMFLOAT2 uv;
};

struct Triangle
{
	Vertex v1, v2, v3;
};

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
	static bool first = true;
	static std::vector<Triangle> triangles;

	if (first)
	{
		first = false;
		for (int dq = 0; dq < p_drawQueue.size(); dq++)
		{
			for (int m = 0; m < p_drawQueue[dq].ObjectData.size(); m++)
			{
				StaticMesh * mesh = p_drawQueue[dq].MeshPtr;
				Triangle t;
				for (int v = 0; v < mesh->GetRawVertices()->size(); v+=3)
				{
					DirectX::XMFLOAT4X4 world = p_drawQueue[dq].ObjectData[m].WorldMatrix;
					
					DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&world));

					t.v1.pos = mesh->GetRawVertices()->at(v).Position;
					t.v1.normal = mesh->GetRawVertices()->at(v).Normal;
					t.v1.uv = mesh->GetRawVertices()->at(v).UV;


					DirectX::XMStoreFloat4(&t.v1.pos, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat4(&t.v1.pos), worldMatrix));
					DirectX::XMStoreFloat4(&t.v1.normal, DirectX::XMVector3Normalize(DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat4(&t.v1.pos), worldMatrix)));

					t.v2.pos = mesh->GetRawVertices()->at(v + 1).Position;
					t.v2.normal = mesh->GetRawVertices()->at(v + 1).Normal;
					t.v2.uv = mesh->GetRawVertices()->at(v + 1).UV;


					DirectX::XMStoreFloat4(&t.v2.pos, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat4(&t.v2.pos), worldMatrix));
					DirectX::XMStoreFloat4(&t.v2.normal, DirectX::XMVector3Normalize(DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat4(&t.v2.pos), worldMatrix)));

					t.v3.pos = mesh->GetRawVertices()->at(v + 2).Position;
					t.v3.normal = mesh->GetRawVertices()->at(v + 2).Normal;
					t.v3.uv = mesh->GetRawVertices()->at(v + 2).UV;


					DirectX::XMStoreFloat4(&t.v3.pos, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat4(&t.v3.pos), worldMatrix));
					DirectX::XMStoreFloat4(&t.v3.normal, DirectX::XMVector3Normalize(DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat4(&t.v3.pos), worldMatrix)));

					triangles.push_back(t);
				}
			}
		}
	}

	p_drawQueue.clear();

	UINT * rayCounter = nullptr;

	if (FAILED(m_counterStencil->Read(rayCounter)))
		return;
	m_counterStencil->Unmap();

	DirectX::XMFLOAT4 camPos = Camera::GetActiveCamera()->GetPosition();
	DirectX::XMFLOAT4 camDir = Camera::GetActiveCamera()->GetDirectionVector();


	POINT windowSize = p_window->GetWindowSize();

	RAY_BOX data;
	
	data.viewerPosViewSpace.x = (float)windowSize.x * 0.5f;
	data.viewerPosViewSpace.y = (float)windowSize.y * 0.5f;
	data.viewerPosViewSpace.z = -(data.viewerPosViewSpace.x / tan(Camera::GetActiveCamera()->GetFOV()));
	data.viewerPosViewSpace.w = 1.0f;
	
	data.viewerPos.x = camPos.x;
	data.viewerPos.y = camPos.y;
	data.viewerPos.z = camPos.z;
	data.viewerPos.w = 1.0f;

	data.cameraDir.x = camDir.x;
	data.cameraDir.y = camDir.y;
	data.cameraDir.z = camDir.z;
	data.cameraDir.w = 0.0f;

	data.index.x = windowSize.x;
	data.index.y = windowSize.y;
	data.index.z = triangles.size();
	
	data.viewMatrixInverse = Camera::GetActiveCamera()->GetViewProjectionMatrix();
	data.projMatrixInverse = Camera::GetActiveCamera()->GetProjectionMatrix();

	DirectX::XMStoreFloat4x4A(&data.projMatrixInverse,
		DirectX::XMMatrixInverse(nullptr,
			DirectX::XMLoadFloat4x4A(&data.projMatrixInverse)));

	DirectX::XMStoreFloat4x4A(&data.viewMatrixInverse,
		DirectX::XMMatrixInverse(nullptr,
		DirectX::XMLoadFloat4x4A(&data.viewMatrixInverse)));
	
	// TODO :: FENCE
	Sleep(200);

	UINT c = 0;
	if (rayCounter)
		c = rayCounter[0];
	if (c == 0)
		return;
	OpenCommandList(m_pipelineState);
	const UINT frameIndex = p_coreRender->GetFrameIndex();

	p_coreRender->SetResourceDescriptorHeap(p_commandList[frameIndex]);
	p_commandList[frameIndex]->SetComputeRootSignature(m_rootSignature);

	m_squareIndex.SetData(&data, sizeof(data));
	m_squareIndex.BindComputeShader(RAY_SQUARE_INDEX, p_commandList[frameIndex]);
	m_rayTexture.BindComputeShader(RAY_TEXTURE, p_commandList[frameIndex]);

	m_rayStencil->BindCompute(RAY_INDICES, p_commandList[frameIndex]);

	m_meshTriangles.SetData(triangles.data(), triangles.size() * sizeof(Triangle));
	m_meshTriangles.BindComputeShader(TRIANGLES, p_commandList[frameIndex]);


	p_commandList[frameIndex]->Dispatch(*rayCounter, 1, 1);

	_ExecuteCommandList();

	Sleep(200);

	p_coreRender->GetDeferredPass()->SetRayData(&m_rayTexture);

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
	data.index.x = windowSize.x;
	data.index.y = windowSize.y;
	data.index.z = windowSize.x;
	data.index.w = windowSize.y;

	m_squareIndex.SetData(&data, sizeof(data));
	m_squareIndex.BindComputeShader(RAY_SQUARE_INDEX, p_commandList[frameIndex]);
	m_rayTexture.BindComputeShader(RAY_TEXTURE, p_commandList[frameIndex]);

	POINT wndSize = Window::GetInstance()->GetWindowSize();

	p_commandList[frameIndex]->Dispatch(
		UINT((wndSize.x / 32.0f) + 0.5f),	// We need equal or more threads then we have pixels
		UINT((wndSize.y / 32.0f) + 0.5f),	// We need equal or more threads then we have pixels
		1);
	

	_ExecuteCommandList();
}

void ComputePass::SetGeometryData(RenderTarget * const * renderTargets, const UINT & size)
{
	this->m_geometryRenderTargetsSize = size;
	m_geometryRenderTargets = renderTargets;
}

void ComputePass::SetRayData(UAV * rayStencil, UAV * counterStencil)
{
	m_rayStencil = rayStencil;
	m_counterStencil = counterStencil;
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
	
	if (FAILED(hr = m_rayTexture.Init(L"RayTexture", 0, 0, 1, DXGI_FORMAT_R32G32B32A32_FLOAT)))
	{
		return hr;
	}

	if (FAILED(hr = m_meshTriangles.Init(sizeof(Triangle) * 1024 * 12, L"TriMeshData", ConstantBuffer::STRUCTURED_BUFFER, sizeof(Triangle))))
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

	CD3DX12_ROOT_PARAMETER1 rootParameters[4];
	rootParameters[RAY_SQUARE_INDEX].InitAsConstantBufferView(0);
	rootParameters[RAY_TEXTURE].InitAsDescriptorTable(1,&descRange);
	rootParameters[RAY_INDICES].InitAsUnorderedAccessView(1);
	rootParameters[TRIANGLES].InitAsShaderResourceView(0);


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
