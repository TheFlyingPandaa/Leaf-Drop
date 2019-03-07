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
#define LIGHT_BUFFER		6
#define OCTREE				7
#define MESH_ARRAY			8

#define MESH_ARRAY_SPACE	3
#define TEXTURE_SPACE		4

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
	static bool first = true;

	if (first)
	{
		first = false;
		for (int dq = 0; dq < p_staticDrawQueue.size(); dq++)
		{
			for (int m = 0; m < p_staticDrawQueue[dq].DrawableObjectData.size(); m++)
			{
				DirectX::XMFLOAT4X4A WorldInverse = p_staticDrawQueue[dq].DrawableObjectData[m].WorldMatrix;
				DirectX::XMStoreFloat4x4A(&WorldInverse, DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4A(&WorldInverse)))));
				auto aabb = p_staticDrawQueue[dq].MeshPtr->GetAABB();
				STRUCTS::MeshValues ocv;
				ocv.World = p_staticDrawQueue[dq].DrawableObjectData[m].WorldMatrix;
				ocv.WorldInverse = WorldInverse;
				ocv.MeshIndex = aabb.meshIndex;
				ocv.Min = aabb.min;
				ocv.Max = aabb.max;
				ocv.TextureIndex = p_staticDrawQueue[dq].TextureOffset;
				m_staticOctreeValues.push_back(ocv);
			}
		}

		m_staticOcTree.BuildTree(-512, -512, -512, 3u, 1024u);
		m_dynamicOcTree.BuildTree(-512, -512, -512, 3u, 1024u);
		m_dynamicOcTree.CreateBuffer(L"Ray_Octree");
		m_staticOcTree.PlaceObjects(m_staticOctreeValues);

		// http://dcgi.fel.cvut.cz/home/havran/ARTICLES/sccg2011.pdf
		// http://gpupro.blogspot.com/2013/01/bit-trail-traversal-for-stackless-lbvh-on-directcompute.html
	}

	static double mergeTime = 0;
	static double copyTime = 0;
	static int timeCounter = 0;

	Timer t;
	t.Start(); 
	
	m_dynamicOctreeValues.clear();
	for (int dq = 0; dq < p_dynamicDrawQueue.size(); dq++)
	{
		for (int m = 0; m < p_dynamicDrawQueue[dq].DrawableObjectData.size(); m++)
		{
			DirectX::XMFLOAT4X4A WorldInverse = p_dynamicDrawQueue[dq].DrawableObjectData[m].WorldMatrix;
			DirectX::XMStoreFloat4x4A(&WorldInverse, DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4A(&WorldInverse)))));
			auto aabb = p_dynamicDrawQueue[dq].MeshPtr->GetAABB();
			STRUCTS::MeshValues ocv;
			ocv.World = p_dynamicDrawQueue[dq].DrawableObjectData[m].WorldMatrix;
			ocv.WorldInverse = WorldInverse;
			ocv.MeshIndex = aabb.meshIndex;
			ocv.Min = aabb.min;
			ocv.Max = aabb.max;
			ocv.TextureIndex = p_dynamicDrawQueue[dq].TextureOffset;
			m_dynamicOctreeValues.push_back(ocv);
		}
	}

	m_dynamicOcTree.PlaceObjects(m_dynamicOctreeValues, true);
	m_dynamicOcTree.Merge(m_staticOcTree);
	mergeTime += t.Stop(Timer::MILLISECONDS);

	auto tree = m_dynamicOcTree.GetTree();
	size_t size = tree.size();

	UINT currentOffset = 0;

	for (size_t i = 0; i < size; i++)
	{
		UINT sizeofMeshInd = (UINT)tree[i].meshDataIndices.size() * sizeof(UINT);
		m_ocTreeBuffer.SetData(&tree[i], tree[i].byteSize - sizeofMeshInd, currentOffset, true);
		currentOffset += tree[i].byteSize - sizeofMeshInd;
		m_ocTreeBuffer.SetData(tree[i].meshDataIndices.data(), sizeofMeshInd, currentOffset, true);
		currentOffset += sizeofMeshInd;
	}


	copyTime += t.Stop(Timer::MILLISECONDS);

	if (timeCounter++ == 1000)
	{
		std::ofstream lol;
		lol.open("MergeTime.txt");
		lol << "Average time to merge tree: " << mergeTime / 1000 << " ms\n";
		lol << "Average time to copy tree: " << copyTime / 1000 << " ms\n";
		lol.close();
	}




}


void ComputePass::Draw()
{

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
	data.info.z = (UINT)m_staticOctreeValues.size() + (UINT)m_dynamicOctreeValues.size();
	
	const UINT frameIndex = p_coreRender->GetFrameIndex();

	p_coreRender->BeginCopy();

	m_dynamicOcTree.WriteToBuffer(p_coreRender->GetCopyCommandList(), m_ocTreeBuffer.GetResource());
	// copy constantBuffer
	p_coreRender->EndCopy();

	//Sleep(500);
	m_fence2.WaitForFinnishExecution();


	OpenCommandList(m_pipelineState);
	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(targetResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));
	p_commandList[frameIndex]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ocTreeBuffer.GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	p_coreRender->SetResourceDescriptorHeap(p_commandList[frameIndex]);
	p_commandList[frameIndex]->SetComputeRootSignature(m_rootSignature);



	_SetLightData();

	m_lightUav.BindComputeSrv(LIGHT_TABLE, p_commandList[frameIndex]);
	m_lightsBuffer.BindComputeShader(LIGHT_BUFFER, p_commandList[frameIndex]);

	m_rayTexture.Clear(p_commandList[frameIndex]);

	m_squareIndex.SetData(&data, sizeof(data));
	m_squareIndex.BindComputeShader(RAY_SQUARE_INDEX, p_commandList[frameIndex]);
	m_rayTexture.BindComputeShader(RAY_TEXTURE, p_commandList[frameIndex]);

	m_rayStencil->BindComputeSrv(RAY_INDICES, p_commandList[frameIndex]);

	UINT dataOffset = 0;
	m_meshData.SetData(m_staticOctreeValues.data(), dataOffset = (UINT)m_staticOctreeValues.size() * sizeof(STRUCTS::MeshValues));
	m_meshData.SetData(m_dynamicOctreeValues.data(), (UINT)m_dynamicOctreeValues.size() * sizeof(STRUCTS::MeshValues), dataOffset);
	m_meshData.BindComputeShader(TRIANGLES, p_commandList[frameIndex]);
	m_ocTreeBuffer.BindComputeShader(OCTREE, p_commandList[frameIndex]);

	TextureAtlas::GetInstance()->BindlessComputeSetGraphicsRootDescriptorTable(TEXTURE_ATLAS, p_commandList[frameIndex]);

	StaticMesh::BindCompute(MESH_ARRAY, p_commandList[frameIndex]);

	p_commandList[frameIndex]->Dispatch(data.info.x, data.info.y, 1);

	p_commandList[frameIndex]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_rayTexture.GetResource()));

	p_commandList[frameIndex]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ocTreeBuffer.GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));

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
	m_fence2.Release();

	p_ReleaseCommandList();
	m_squareIndex.Release();
	m_meshData.Release();
	m_rayTexture.Release();
	m_lightUav.Release();
	m_lightsBuffer.Release();
	m_ocTreeBuffer.Release();

	m_dynamicOcTree.Release();
	m_staticOcTree.Release();
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
	if (FAILED(hr = m_fence.CreateFence(m_commandQueue)))
	{
		return hr;
	}

	if (FAILED(hr = m_fence2.CreateFence(p_coreRender->GetCopyQueue())))
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

	if (FAILED(hr = m_meshData.Init(sizeof(STRUCTS::MeshValues) * MAX_OBJECTS, L"MeshDataStructuredBuffer", ConstantBuffer::STRUCTURED_BUFFER, sizeof(STRUCTS::MeshValues))))
	{
		return hr;
	}

	if (FAILED(hr = m_ocTreeBuffer.Init(4096 * 1024, L"OcTrEeBuFfEr", ConstantBuffer::STRUCTURED_BUFFER, 1)))
	{
		return hr;
	}
	const UINT bufferSize = 1024 * 64;
	const UINT elementSize = sizeof(LIGHT_VALUES);
	if (FAILED(hr = m_lightUav.Init(L"Ray Lights", bufferSize, bufferSize / elementSize, elementSize)))
	{
		return hr;
	}

	if (FAILED(hr = m_lightsBuffer.Init(256, L"Ray Light", ConstantBuffer::CBV_TYPE::CONSTANT_BUFFER)))
	{
		return hr;
	}

	m_lightUav.ConstantMap();

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
	rootParameters[LIGHT_BUFFER].InitAsConstantBufferView(0, 2);
	rootParameters[MESH_ARRAY].InitAsDescriptorTable(1, &descRange2);
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

void ComputePass::_SetLightData()
{
	LIGHT_VALUES values;

	PointLight * pl;
	DirectionalLight * dl;

	struct
	{
		UINT a, b, c, d;
	} uint4;

	uint4.a = static_cast<UINT>(p_lightQueue.size());
	m_lightsBuffer.SetData(&uint4, sizeof(UINT) * 4);


	for (UINT i = 0; i < p_lightQueue.size(); i++)
	{
		values.Position = p_lightQueue[i]->GetPosition();
		values.Color = p_lightQueue[i]->GetColor();

		values.Type.x = p_lightQueue[i]->GetType();
		values.Type.y = p_lightQueue[i]->GetType();
		values.Type.z = p_lightQueue[i]->GetType();
		values.Type.w = p_lightQueue[i]->GetType();

		if (pl = dynamic_cast<PointLight*>(p_lightQueue[i]))
		{
			values.Point = DirectX::XMFLOAT4(pl->GetIntensity(), pl->GetDropOff(), pl->GetPow(), pl->GetRadius());
		}
		if (dl = dynamic_cast<DirectionalLight*>(p_lightQueue[i]))
		{
			values.Direction = DirectX::XMFLOAT4(dl->GetDirection().x, dl->GetDirection().y, dl->GetDirection().z, dl->GetIntensity());
		}
		m_lightUav.CopyData(&values, sizeof(LIGHT_VALUES), i * sizeof(LIGHT_VALUES));
	}
}
