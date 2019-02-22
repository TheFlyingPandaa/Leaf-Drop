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
#define TEXTURE_ATLAS	4
#define OCTREE		5

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
	static std::vector<STRUCTS::Triangle> triangles;

	if (first)
	{
		first = false;
		for (int dq = 0; dq < p_drawQueue.size(); dq++)
		{
			for (int m = 0; m < p_drawQueue[dq].DrawableObjectData.size(); m++)
			{
				StaticMesh * mesh = p_drawQueue[dq].MeshPtr;
				STRUCTS::Triangle t;
				for (int v = 0; v < mesh->GetRawVertices()->size(); v+=3)
				{
					DirectX::XMFLOAT4X4 world = p_drawQueue[dq].DrawableObjectData[m].WorldMatrix;
					
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
		m_ocTree.BuildTree(triangles, 4u, 256u);
		// http://dcgi.fel.cvut.cz/home/havran/ARTICLES/sccg2011.pdf
		// http://gpupro.blogspot.com/2013/01/bit-trail-traversal-for-stackless-lbvh-on-directcompute.html

		auto tree = m_ocTree.GetTree();
		size_t size = tree.size();
		UINT currentOffset = 0;
		
		for (size_t i = 0; i < size; i++)
		{
			UINT sizeofTriInd = (UINT)tree[i].triangleIndices.size() * sizeof(UINT);
			m_ocTreeBuffer.SetData(&tree[i], tree[i].byteSize - sizeofTriInd, currentOffset, true);
			currentOffset += tree[i].byteSize - sizeofTriInd;
			m_ocTreeBuffer.SetData(tree[i].triangleIndices.data(), sizeofTriInd, currentOffset, true);
			currentOffset += sizeofTriInd;
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
	data.viewerPos.x = camPos.x;
	data.viewerPos.y = camPos.y;
	data.viewerPos.z = camPos.z;
	data.viewerPos.w = 1.0f;

	data.info.x = windowSize.x;
	data.info.y = windowSize.y;
	data.info.z = (UINT)triangles.size();
	
	const UINT frameIndex = p_coreRender->GetFrameIndex();

	UINT c = 0;
	if (rayCounter)
		c = rayCounter[0];
	OpenCommandList(m_pipelineState);

	m_rayTexture.Clear(p_commandList[frameIndex]);


	p_coreRender->SetResourceDescriptorHeap(p_commandList[frameIndex]);
	p_commandList[frameIndex]->SetComputeRootSignature(m_rootSignature);

	m_squareIndex.SetData(&data, sizeof(data));
	m_squareIndex.BindComputeShader(RAY_SQUARE_INDEX, p_commandList[frameIndex]);
	m_rayTexture.BindComputeShader(RAY_TEXTURE, p_commandList[frameIndex]);

	m_rayStencil->BindComputeSrv(RAY_INDICES, p_commandList[frameIndex]);

	m_meshTriangles.SetData(triangles.data(), (UINT)triangles.size() * sizeof(STRUCTS::Triangle));
	m_meshTriangles.BindComputeShader(TRIANGLES, p_commandList[frameIndex]);
	m_ocTreeBuffer.BindComputeShader(OCTREE, p_commandList[frameIndex]);
	TextureAtlas::GetInstance()->SetMagnusRootDescriptorTable(TEXTURE_ATLAS, p_commandList[frameIndex]);


	p_commandList[frameIndex]->Dispatch(*rayCounter, 1, 1);

	_ExecuteCommandList();
	
	m_fence.WaitForFinnishExecution();

	p_coreRender->GetDeferredPass()->SetRayData(&m_rayTexture);

}

void ComputePass::Release()
{
	SAFE_RELEASE(m_pipelineState);
	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_commandQueue);
	m_fence.Release();
	p_ReleaseCommandList();
	m_squareIndex.Release();
	m_meshTriangles.Release();
	m_rayTexture.Release();
}

void ComputePass::Clear()
{
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
	if (FAILED(hr = m_fence.CreateFence(m_commandQueue)))
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

	if (FAILED(hr = m_meshTriangles.Init(sizeof(STRUCTS::Triangle) * MAX_OBJECTS * 12, L"TriMeshData", ConstantBuffer::STRUCTURED_BUFFER, sizeof(STRUCTS::Triangle))))
	{
		return hr;
	}
	if (FAILED(hr = m_ocTreeBuffer.Init(4096 * 1024, L"OcTrEeBuFfEr", ConstantBuffer::STRUCTURED_BUFFER, 1)))
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
	D3D12_DESCRIPTOR_RANGE1 descRange1 = CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);
	
	CD3DX12_ROOT_PARAMETER1 rootParameters[6];
	rootParameters[RAY_SQUARE_INDEX].InitAsConstantBufferView(0);
	rootParameters[RAY_TEXTURE].InitAsDescriptorTable(1,&descRange);
	rootParameters[RAY_INDICES].InitAsShaderResourceView(1);
	rootParameters[TRIANGLES].InitAsShaderResourceView(0);
	rootParameters[TEXTURE_ATLAS].InitAsDescriptorTable(1, &descRange1);
	rootParameters[OCTREE].InitAsShaderResourceView(0, 1);


	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);
	rootSignatureDesc.Init_1_1(
		_countof(rootParameters),
		rootParameters,
		1,
		&samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
	);

	ID3DBlob * signature = nullptr;
	ID3DBlob * error = nullptr;
	if (SUCCEEDED(hr = D3D12SerializeRootSignature(&rootSignatureDesc.Desc_1_0,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signature,
		&error)))
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
	else
	{
		OutputDebugStringA(static_cast<char*>(error->GetBufferPointer()));
		error->Release();
	}


	//if (FAILED(hr = _InitClearRootSignature()))
	//{
	//	return hr;
	//}

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
