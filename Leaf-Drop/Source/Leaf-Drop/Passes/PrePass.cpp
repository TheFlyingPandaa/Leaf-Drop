#include "CorePCH.h"
#include "PrePass.h"
#include "../Objects/Camera.h"
#include "../Wrappers/ShaderCreator.h"
#include "../Objects/Drawable.h"
#include "../Objects/StaticMesh.h"

#define CAMERA_BUFFER	0
#define WORLD_MATRICES	1

PrePass::PrePass()
{
}


PrePass::~PrePass()
{
}

HRESULT PrePass::Init()
{
	HRESULT hr = 0;

	if (FAILED(hr = _Init()))
	{
		return hr;
	}

	return hr;
}

void PrePass::Update()
{
	if (FAILED(OpenCommandList(m_pipelineState)))
	{
		return;
	}

	const UINT frameIndex = p_coreRender->GetFrameIndex();
	ID3D12GraphicsCommandList * commandList = p_commandList[frameIndex];
	commandList->SetGraphicsRootSignature(m_rootSignature);

	D3D12_CPU_DESCRIPTOR_HANDLE handle = p_coreRender->GetCPUDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	const SIZE_T resourceSize = p_coreRender->GetResourceDescriptorHeapSize();
	   
	m_depthBuffer.SwapToDSV(commandList);

	int counter = 0;
	for (size_t i = 0; i < p_staticDrawQueue.size(); i++)
	{
		for (size_t k = 0; k < p_staticDrawQueue[i].DrawableObjectData.size(); k++)
		{
			auto world = p_staticDrawQueue[i].DrawableObjectData[k];
			m_worldMatrices.SetData(&world, sizeof(world), sizeof(world) * (counter++));
		}
	}
	for (size_t i = 0; i < p_dynamicDrawQueue.size(); i++)
	{
		for (size_t k = 0; k < p_dynamicDrawQueue[i].DrawableObjectData.size(); k++)
		{
			auto world = p_dynamicDrawQueue[i].DrawableObjectData[k];
			m_worldMatrices.SetData(&world, sizeof(world), sizeof(world) * (counter++));
		}
	}


	m_worldMatrices.Bind(WORLD_MATRICES, commandList);

	Camera * cam = Camera::GetActiveCamera();
	cam->Update();

	DirectX::XMFLOAT4X4A viewProj = cam->GetViewProjectionMatrix();

	m_camBuffer.SetData(&viewProj, sizeof(DirectX::XMFLOAT4X4A));
	m_camBuffer.Bind(CAMERA_BUFFER, commandList);

}

void PrePass::Draw()
{
	const UINT frameIndex = p_coreRender->GetFrameIndex();
	ID3D12GraphicsCommandList * commandList = p_commandList[frameIndex];

	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandles;
	const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
	{ m_renderTarget.GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr +
	frameIndex *
	m_renderTarget.GetRenderTargetDescriptorHeapSize() };
	renderTargetHandles = rtvHandle;

	commandList->OMSetRenderTargets(RENDER_TARGETS, &renderTargetHandles, FALSE, &m_depthBuffer.GetHandle());

	m_depthBuffer.Clear(commandList);

	
	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT offset = 0;

	for (size_t i = 0; i < p_staticDrawQueue.size(); i++)
	{
		m_worldMatrices.Bind(WORLD_MATRICES, commandList, offset);

		offset += p_staticDrawQueue[i].DrawableObjectData.size() * sizeof(InstanceGroup::ObjectDataStruct);

		StaticMesh * m = p_staticDrawQueue[i].MeshPtr;
		commandList->IASetVertexBuffers(0, 1, &m->GetVBV());
		commandList->DrawInstanced(m->GetNumberOfVertices(), (UINT)p_staticDrawQueue[i].DrawableObjectData.size(), 0, 0);
	}

	for (size_t i = 0; i < p_dynamicDrawQueue.size(); i++)
	{
		m_worldMatrices.Bind(WORLD_MATRICES, commandList, offset);

		offset += p_dynamicDrawQueue[i].DrawableObjectData.size() * sizeof(InstanceGroup::ObjectDataStruct);

		StaticMesh * m = p_dynamicDrawQueue[i].MeshPtr;
		commandList->IASetVertexBuffers(0, 1, &m->GetVBV());
		commandList->DrawInstanced(m->GetNumberOfVertices(), (UINT)p_dynamicDrawQueue[i].DrawableObjectData.size(), 0, 0);
	}


	m_depthBuffer.SwapToSRV(commandList);


	ExecuteCommandList();

	// Possible fence?? boobie fix

	p_coreRender->GetGeometryPass()->SetDepthPreBuffer(&m_depthBuffer);

}

void PrePass::Release()
{
	m_camBuffer.Release();
	m_worldMatrices.Release();
	m_renderTarget.Release();
	m_depthBuffer.Release();
	p_ReleaseCommandList();

	

	SAFE_RELEASE(m_pipelineState);
	SAFE_RELEASE(m_rootSignature);
}

HRESULT PrePass::_Init()
{
	HRESULT hr = 0;

	if (FAILED(hr = p_CreateCommandList(L"PrePass")))
	{
		return hr;
	}
	if (FAILED(hr = _InitRootSignature()))
	{
		return hr;
	}
	if (FAILED(hr = _InitShader()))
	{
		return hr;
	}
	if (FAILED(hr = _InitPipelineState()))
	{
		return hr;
	}
	if (FAILED(hr = m_camBuffer.Init(sizeof(DirectX::XMFLOAT4X4), L"PrePass Camera")))
	{
		return hr;
	}
	if (FAILED(hr = m_worldMatrices.Init(MAX_OBJECTS * sizeof(IRender::InstanceGroup::ObjectDataStruct), L"PrePass Matrix", ConstantBuffer::CBV_TYPE::STRUCTURED_BUFFER, sizeof(IRender::InstanceGroup::ObjectDataStruct))))
	{
		return hr;
	}
	if (FAILED(hr = m_renderTarget.Init(L"PrePass")))
	{
		return hr;
	}
	if (FAILED(hr = m_depthBuffer.Init(L"PrePass",0,0,1,TRUE)))
	{
		return hr;
	}
	_CreateViewPort();
	return hr;
}

HRESULT PrePass::_InitRootSignature()
{
	HRESULT hr = 0;

	CD3DX12_ROOT_PARAMETER1 rootParameters[2];
	rootParameters[CAMERA_BUFFER].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[WORLD_MATRICES].InitAsShaderResourceView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;


	rootSignatureDesc.Init_1_1(
		_countof(rootParameters),
		rootParameters,
		NULL,
		nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		//D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
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

HRESULT PrePass::_InitShader()
{
	HRESULT hr = 0;
	ID3DBlob * blob = nullptr;

	if (FAILED(hr = ShaderCreator::CreateShader(L"..\\Leaf-Drop\\Source\\Leaf-Drop\\Shaders\\PrePass\\PrePassVertex.hlsl", blob, "vs_5_1")))
	{
		return hr;
	}
	m_vertexShader.pShaderBytecode = blob->GetBufferPointer();
	m_vertexShader.BytecodeLength = blob->GetBufferSize();

	return hr;
}

HRESULT PrePass::_InitPipelineState()
{
	HRESULT hr = 0;
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	m_inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	m_inputLayoutDesc.pInputElementDescs = inputLayout;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.InputLayout = m_inputLayoutDesc;
	graphicsPipelineStateDesc.pRootSignature = m_rootSignature;
	graphicsPipelineStateDesc.VS = m_vertexShader;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	graphicsPipelineStateDesc.SampleMask = 0xffffffff;
	graphicsPipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	graphicsPipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	graphicsPipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	DXGI_SWAP_CHAIN_DESC desc;
	if (FAILED(hr = p_coreRender->GetSwapChain()->GetDesc(&desc)))
	{
		return hr;
	}
	graphicsPipelineStateDesc.SampleDesc = desc.SampleDesc;

	if (FAILED(hr = p_coreRender->GetDevice()->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&m_pipelineState))))
	{
		SAFE_RELEASE(m_pipelineState);
	}


	return hr;
}

void PrePass::_CreateViewPort()
{
	POINT wndSize = p_window->GetWindowSize();
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = (FLOAT)wndSize.x;
	m_viewport.Height = (FLOAT)wndSize.y;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = wndSize.x;
	m_scissorRect.bottom = wndSize.y;
}