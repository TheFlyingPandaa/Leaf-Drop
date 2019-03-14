#include "CorePCH.h"
#include "ComputePass.h"
#include "../Wrappers/ShaderCreator.h"
#include "../Objects/Camera.h"
#include "../Objects/StaticMesh.h"

#include "Source/Leaf-Drop/Objects/Lights/PointLight.h"
#include "Source/Leaf-Drop/Objects/Lights/DirectionalLight.h"

#include "../Utillity/Timer.h"


#define RAY_SQUARE_INDEX	0
#define RAY_TEXTURE			1
#define RAY_INDICES			2
#define TRIANGLES			3
#define TEXTURE_ATLAS		4
#define LIGHT_TABLE			5
#define OCTREE				6
#define MESH_ARRAY			7
#define OFFSET_BUFFER		8

#define MESH_ARRAY_SPACE	3
#define TEXTURE_SPACE		4

ComputePass::ComputePass()
{
	timer.OpenLog("Compute.txt");
}

ComputePass::~ComputePass()
{
	timer.CloseLog();
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
	struct UINT4
	{
		float x, y, z, w;

	} offset;
		

	int counter = 0;
	for (int y = 0; y < DISPATCH_MUL; y++)
	{
		for (int x = 0; x < DISPATCH_MUL; x++)
		{
			offset.x = (float)x / (DISPATCH_MUL);
			offset.y = (float)y / (DISPATCH_MUL);

			m_offsetBuffer.SetData(&offset, sizeof(UINT4), counter++ * sizeof(UINT4));
		}
	}
}


void ComputePass::Draw()
{
	p_coreRender->GetFence(DEFINE)->Wait(m_commandQueue);
	p_coreRender->GetFence(UPDATE)->Wait(m_commandQueue);

	UpdatePass * up = p_coreRender->GetUpdatePass();

	UpdatePass::RayData rayData = up->GetRayData();

	DirectX::XMFLOAT4 camPos = Camera::GetActiveCamera()->GetPosition();
	DirectX::XMFLOAT4 camDir = Camera::GetActiveCamera()->GetDirectionVector();

	POINT windowSize = p_window->GetWindowSize();

	RAY_BOX data;
	data.viewerPos.x = camPos.x;
	data.viewerPos.y = camPos.y;
	data.viewerPos.z = camPos.z;
	data.viewerPos.w = 1.0f;

	data.info.x = windowSize.x / SCREEN_DIV;
	data.info.y = windowSize.y / SCREEN_DIV;
	data.info.z = rayData.NumberOfObjectsInTree;
	
	const UINT frameIndex = p_coreRender->GetFrameIndex();


	OpenCommandList(m_pipelineState);
	p_commandList[frameIndex]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rayData.OctreeBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	p_coreRender->SetResourceDescriptorHeap(p_commandList[frameIndex]);
	p_commandList[frameIndex]->SetComputeRootSignature(m_rootSignature);

	rayData.LightBuffer->BindComputeShader(LIGHT_TABLE, p_commandList[frameIndex]);
	
	m_rayTexture.Clear(p_commandList[frameIndex]);

	m_squareIndex.SetData(&data, sizeof(data));
	m_squareIndex.BindComputeShader(RAY_SQUARE_INDEX, p_commandList[frameIndex]);
	m_rayTexture.BindComputeShader(RAY_TEXTURE, p_commandList[frameIndex]);

	m_rayStencil->BindComputeSrv(RAY_INDICES, p_commandList[frameIndex]);

	rayData.ObjectData->BindComputeShader(TRIANGLES, p_commandList[frameIndex]);
	rayData.OctreeBuffer->BindComputeShader(OCTREE, p_commandList[frameIndex]);

	TextureAtlas::GetInstance()->BindlessComputeSetGraphicsRootDescriptorTable(TEXTURE_ATLAS, p_commandList[frameIndex]);

	StaticMesh::BindCompute(MESH_ARRAY, p_commandList[frameIndex]);

	int counter = 0;
	for (int y = 0; y < DISPATCH_MUL; y++)
	{
		for (int x = 0; x < DISPATCH_MUL; x++)
		{
			m_offsetBuffer.BindComputeShader(OFFSET_BUFFER, p_commandList[frameIndex], counter++ * sizeof(UINT) * 4);
			p_commandList[frameIndex]->Dispatch(data.info.x / DISPATCH_MUL, data.info.y / DISPATCH_MUL, 1);
		}
	}

	p_commandList[frameIndex]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_rayTexture.GetResource()));

	p_commandList[frameIndex]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rayData.OctreeBuffer->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));

	_ExecuteCommandList();
	
	p_coreRender->GetDeferredPass()->SetRayData(&m_rayTexture);

	p_coreRender->GetFence(RAY_TRACING)->Signal(m_commandQueue);
}

void ComputePass::Release()
{
	SAFE_RELEASE(m_pipelineState);
	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_commandQueue);
	
	p_ReleaseCommandList();
	m_squareIndex.Release();
	m_rayTexture.Release();
	m_offsetBuffer.Release();
}

void ComputePass::Clear()
{
	IRender::Clear();
}

void ComputePass::SetGeometryData(RenderTarget * const * renderTargets, const UINT & size)
{
	this->m_geometryRenderTargetsSize = size;
	m_geometryRenderTargets = renderTargets;
}

void ComputePass::SetRayData(UAV * rayStencil)
{
	m_rayStencil = rayStencil;
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

	if (FAILED(hr = m_squareIndex.Init(sizeof(RAY_BOX), L"RaySqueare", ConstantBuffer::CONSTANT_BUFFER, sizeof(RAY_BOX))))
	{
		return hr;
	}
	
	auto size = p_window->GetWindowSize();

	if (FAILED(hr = m_rayTexture.Init(L"RayTexture", size.x / SCREEN_DIV, size.y / SCREEN_DIV, 1, DXGI_FORMAT_R32G32B32A32_FLOAT)))
	{
		return hr;
	}

	if (FAILED(hr = m_offsetBuffer.Init(1024 * 64, L"Ray offset", ConstantBuffer::CBV_TYPE::STRUCTURED_BUFFER, sizeof(UINT) * 4)))
		return hr;

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

	D3D12_DESCRIPTOR_RANGE1 descRange = CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);
	D3D12_DESCRIPTOR_RANGE1 descRange1 = CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, -1, 0, TEXTURE_SPACE);
	D3D12_DESCRIPTOR_RANGE1 descRange2 = CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, -1, 0, MESH_ARRAY_SPACE, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
	
	

	CD3DX12_ROOT_PARAMETER1 rootParameters[9];
	rootParameters[RAY_SQUARE_INDEX].InitAsConstantBufferView(0);
	rootParameters[RAY_TEXTURE].InitAsDescriptorTable(1,&descRange);
	rootParameters[RAY_INDICES].InitAsShaderResourceView(1);
	rootParameters[TRIANGLES].InitAsShaderResourceView(0);
	rootParameters[TEXTURE_ATLAS].InitAsDescriptorTable(1, &descRange1);
	rootParameters[OCTREE].InitAsShaderResourceView(0, 1);
	rootParameters[LIGHT_TABLE].InitAsShaderResourceView(0, 2);
	rootParameters[MESH_ARRAY].InitAsDescriptorTable(1, &descRange2);
	//rootParameters[OFFSET_BUFFER].InitAsConstantBufferView(1);
	rootParameters[OFFSET_BUFFER].InitAsShaderResourceView(0, 5);
	//rootParameters[MESH_ARRAY].InitAsShaderResourceView(0, 3);
	
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