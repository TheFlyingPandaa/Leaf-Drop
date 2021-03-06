#include "CorePCH.h"
#include "GeometryPass.h"
#include "../Wrappers/ShaderCreator.h"
#include "../Objects/Drawable.h"
#include "../Objects/StaticMesh.h"
#include <EASTL/vector.h>
#include "../Objects/Texture.h"
#include "../Objects/Camera.h"
#include <string>

#include "Source/Leaf-Drop/Objects/Lights/DirectionalLight.h"
#include "Source/Leaf-Drop/Objects/Lights/PointLight.h"


#define CAMERA_BUFFER	0
#define WORLD_MATRICES	1
#define TEXTURE_TABLE	2
#define TEXTURE_INDEX	3
#define LIGHT_ARRAY		4
#define LIGHT_INDEX		5


struct UINT4
{

	UINT x, y, z, w;

};

GeometryPass::GeometryPass() : IRender()
{
}

GeometryPass::~GeometryPass()
{
}

HRESULT GeometryPass::Init()
{
	HRESULT hr = 0;
	if (FAILED(hr = p_CreateCommandList(L"Geometry")))
	{
		return hr;
	}
	if (FAILED(hr = _Init()))
	{
		return hr;
	}

	if (FAILED(hr = m_camBuffer.Init(sizeof(DirectX::XMFLOAT4X4),L"Geometry Camera")))
	{
		return hr;
	}


	if (FAILED(hr = m_worldMatrices.Init(MAX_OBJECTS * sizeof(InstanceGroup::ObjectDataStruct), L"Geometry Matrix", ConstantBuffer::CBV_TYPE::STRUCTURED_BUFFER, sizeof(InstanceGroup::ObjectDataStruct))))
	{
		return hr;
	}
	if (FAILED(hr = m_textureIndex.Init(1024 * 64, L"Geometry Index", ConstantBuffer::CBV_TYPE::STRUCTURED_BUFFER, sizeof(UINT4))))
	{
		
		return hr;
	}
	if (FAILED(hr = m_depthBuffer.Init(L"Geometry",0,0,1,FALSE, DXGI_FORMAT_D32_FLOAT)))
	{
		return hr;
	}

	for (UINT i = 0; i < RENDER_TARGETS; i++)
	{
		m_renderTarget[i] = new RenderTarget();
		if (FAILED(hr = m_renderTarget[i]->Init(L"Geometry", 0, 0, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, true)))
		{
			return hr;
		}
	}
	m_ptrAtlas = TextureAtlas::GetInstance();

	const UINT bufferSize = 1024 * 64;
	const UINT elementSize = sizeof(STRUCTS::LIGHT_VALUES);

	if (FAILED(hr = m_lightsArrayBuffer.Init(bufferSize, L"Geometry Light array", ConstantBuffer::CBV_TYPE::STRUCTURED_BUFFER, elementSize)))
		return hr;
	if (FAILED(hr = m_lightIndexBuffer.Init(256, L"Geometry Light index", ConstantBuffer::CBV_TYPE::CONSTANT_BUFFER)))
		return hr;
	sTimer[GEOMETRY].SetCommandQueue(p_coreRender->GetCommandQueue());

	return hr;
}

void GeometryPass::Update()
{
	//p_coreRender->GetFence(PRE_PASS)->Wait(p_coreRender->GetCommandQueue());


	m_ptrAtlas->Reset();
	if (FAILED(OpenCommandList()))
	{
		return;
	}

	const UINT frameIndex = p_coreRender->GetFrameIndex();
	ID3D12GraphicsCommandList * commandList = p_commandList[frameIndex];
	
	sTimer[GEOMETRY].Start(p_commandList[frameIndex], sCounter[GEOMETRY]);

	p_coreRender->SetResourceDescriptorHeap(commandList);

	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandles[RENDER_TARGETS];

	for (UINT i = 0; i < RENDER_TARGETS; i++)
	{
		m_renderTarget[i]->SwitchToRTV(commandList);
	
		const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
		{ m_renderTarget[i]->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart().ptr +
		frameIndex *
		m_renderTarget[i]->GetRenderTargetDescriptorHeapSize() };

		commandList->ClearRenderTargetView(rtvHandle, CLEAR_COLOR, 0, nullptr);
		renderTargetHandles[i] = rtvHandle;

	}

	
	commandList->OMSetRenderTargets(RENDER_TARGETS, renderTargetHandles, FALSE, &m_ptrDepthPreBuffer->GetHandle());
	
	commandList->SetPipelineState(m_pipelineState);
	commandList->SetGraphicsRootSignature(m_rootSignature);
	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	int counter = 0;
	UINT textureCounter = 0;
	UINT textureIndexOffset = 0;
	UINT4 textureOffset{ 0,0,0,0 };

	for (size_t i = 0; i < p_staticDrawQueue.size(); i++)
	{
		for (size_t k = 0; k < p_staticDrawQueue[i].DrawableObjectData.size(); k++)
		{
			auto world = p_staticDrawQueue[i].DrawableObjectData[k];
			m_worldMatrices.SetData(&world, sizeof(world), sizeof(world) * (counter++));
		}

		textureOffset.x = (UINT)i * 3;
		textureOffset.y = 3;

		m_textureIndex.SetData(&textureOffset, sizeof(UINT4), sizeof(UINT4) * textureIndexOffset++);
	}
	for (size_t i = 0; i < p_dynamicDrawQueue.size(); i++)
	{
		for (size_t k = 0; k < p_dynamicDrawQueue[i].DrawableObjectData.size(); k++)
		{
			auto world = p_dynamicDrawQueue[i].DrawableObjectData[k];
			m_worldMatrices.SetData(&world, sizeof(world), sizeof(world) * (counter++));
		}

		
		textureOffset.x = (UINT)i * 3 + (p_staticDrawQueue.size() * 3);
		textureOffset.y = 3;
		m_textureIndex.SetData(&textureOffset, sizeof(UINT4), sizeof(UINT4) * textureIndexOffset++);
	}

	m_worldMatrices.Bind(WORLD_MATRICES, commandList);

	Camera * cam = Camera::GetActiveCamera();
	cam->Update();

	DirectX::XMFLOAT4X4A viewProj = cam->GetViewProjectionMatrix();
	DirectX::XMFLOAT4 position = cam->GetPosition();


	m_camBuffer.SetData(&viewProj, sizeof(DirectX::XMFLOAT4X4A));
	m_camBuffer.SetData(&position, sizeof(DirectX::XMFLOAT4), sizeof(DirectX::XMFLOAT4X4A));
	m_camBuffer.Bind(CAMERA_BUFFER, commandList);

	_CopyLightData();
	m_lightsArrayBuffer.Bind(LIGHT_ARRAY, commandList);
	m_lightIndexBuffer.Bind(LIGHT_INDEX, commandList);

}

void GeometryPass::Draw()
{
	const UINT frameIndex = p_coreRender->GetFrameIndex();
	ID3D12GraphicsCommandList * commandList = p_commandList[frameIndex];

	m_ptrAtlas->BindlessGraphicsSetGraphicsRootDescriptorTable(TEXTURE_TABLE, commandList);


	UINT offset = 0;
	UINT textureIndexOffset = 0;
	for (size_t i = 0; i < p_staticDrawQueue.size(); i++)
	{
		m_textureIndex.Bind(TEXTURE_INDEX, commandList, sizeof(UINT4) * textureIndexOffset++);
		
		m_worldMatrices.Bind(WORLD_MATRICES, commandList, offset);

		offset += p_staticDrawQueue[i].DrawableObjectData.size() * sizeof(InstanceGroup::ObjectDataStruct);
			   

		StaticMesh * m = p_staticDrawQueue[i].MeshPtr;
		commandList->IASetVertexBuffers(0, 1, &m->GetVBV());
		commandList->DrawInstanced(m->GetNumberOfVertices(), (UINT)p_staticDrawQueue[i].DrawableObjectData.size(), 0, 0);
	}

	for (size_t i = 0; i < p_dynamicDrawQueue.size(); i++)
	{
		m_textureIndex.Bind(TEXTURE_INDEX, commandList, sizeof(UINT4) * textureIndexOffset++);

		m_worldMatrices.Bind(WORLD_MATRICES, commandList, offset);

		offset += p_dynamicDrawQueue[i].DrawableObjectData.size() * sizeof(InstanceGroup::ObjectDataStruct);


		StaticMesh * m = p_dynamicDrawQueue[i].MeshPtr;
		commandList->IASetVertexBuffers(0, 1, &m->GetVBV());

		commandList->DrawInstanced(m->GetNumberOfVertices(), (UINT)p_dynamicDrawQueue[i].DrawableObjectData.size(), 0, 0);
	}


	for (UINT i = 0; i < RENDER_TARGETS; i++)
	{
		m_renderTarget[i]->SwitchToSRV(commandList);
	}


	sTimer[GEOMETRY].Stop(p_commandList[frameIndex], sCounter[GEOMETRY]);
	sTimer[GEOMETRY].ResolveQueryToCPU(p_commandList[frameIndex], sCounter[GEOMETRY]++);

	ExecuteCommandList();
	
	p_coreRender->GetDeferredPass()->SetGeometryData(m_renderTarget, RENDER_TARGETS);

	p_coreRender->GetFence(GEOMETRY)->Signal(p_coreRender->GetCommandQueue());	
}

void GeometryPass::Release()
{
	p_ReleaseCommandList();
	for (UINT i = 0; i < RENDER_TARGETS; i++)
	{
		m_renderTarget[i]->Release();
		SAFE_DELETE(m_renderTarget[i]);
	}

	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_pipelineState);

	m_depthBuffer.Release();
	m_lightsArrayBuffer.Release();
	m_lightIndexBuffer.Release();

	m_camBuffer.Release();
	m_worldMatrices.Release();
	m_textureIndex.Release();
}

void GeometryPass::SetDepthPreBuffer(DepthBuffer * depthBuffer)
{
	m_ptrDepthPreBuffer = depthBuffer;
}

UAV * GeometryPass::GetUAV()
{
	return nullptr;
}

HRESULT GeometryPass::_InitRootSignature()
{
	HRESULT hr = 0;
	
	CD3DX12_ROOT_PARAMETER1 rootParameters[6];

	rootParameters[CAMERA_BUFFER].InitAsConstantBufferView(0, 0);
	
	D3D12_DESCRIPTOR_RANGE1 descRange = CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, -1, 0, 1);
	rootParameters[TEXTURE_TABLE].InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);
	

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;

	rootParameters[WORLD_MATRICES].InitAsShaderResourceView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);

	
	//rootParameters[TEXTURE_INDEX].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[TEXTURE_INDEX].InitAsShaderResourceView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);

	rootParameters[LIGHT_ARRAY].InitAsShaderResourceView(0, 2, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[LIGHT_INDEX].InitAsConstantBufferView(0, 2, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);


	D3D12_STATIC_SAMPLER_DESC samplers [] = { samplerDesc};


	rootSignatureDesc.Init_1_1(
		_countof(rootParameters),
		rootParameters,
		_countof(samplers),
		samplers,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		//D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS 
		//D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
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
	if (error)
	{
		OutputDebugStringA(static_cast<char*>(error->GetBufferPointer()));
		SAFE_RELEASE(error);
	}
	return hr;
}

HRESULT GeometryPass::_InitShader()
{
	HRESULT hr = 0;
	ID3DBlob * blob = nullptr;
			
	if (FAILED(hr = ShaderCreator::CreateShader(L"..\\Leaf-Drop\\Source\\Leaf-Drop\\Shaders\\GeometryPass\\DefaultGeometryVertex.hlsl", blob, "vs_5_1")))
	{
		return hr;
	}
	m_vertexShader.pShaderBytecode = blob->GetBufferPointer();
	m_vertexShader.BytecodeLength = blob->GetBufferSize();

	std::string Wid(std::to_string(p_window->GetWindowSize().x / SCREEN_DIV));
	std::string DIV(std::to_string(SCREEN_DIV));

	D3D_SHADER_MACRO def[] = {
		"RAY_WIDTH",	Wid.c_str(),
		"RAY_DIV",		DIV.c_str(),
		NULL,NULL};

	if (FAILED(hr = ShaderCreator::CreateShader(L"..\\Leaf-Drop\\Source\\Leaf-Drop\\Shaders\\GeometryPass\\DefaultGeometryPixel.hlsl", blob, "ps_5_1", def)))
	{
		return hr;
	}
	m_pixelShader.pShaderBytecode = blob->GetBufferPointer();
	m_pixelShader.BytecodeLength = blob->GetBufferSize();
	return hr;
}

HRESULT GeometryPass::_InitPipelineState()
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
	graphicsPipelineStateDesc.PS = m_pixelShader;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.NumRenderTargets = RENDER_TARGETS;
	for (UINT i = 0; i < RENDER_TARGETS; i++)
		graphicsPipelineStateDesc.RTVFormats[i] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	graphicsPipelineStateDesc.SampleMask = 0xffffffff;

	D3D12_RASTERIZER_DESC rastDesc{};
	rastDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rastDesc.CullMode = D3D12_CULL_MODE_BACK;
	rastDesc.FrontCounterClockwise = FALSE;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0.0f;
	rastDesc.SlopeScaledDepthBias = 0.0f;
	rastDesc.DepthClipEnable = TRUE;
	rastDesc.MultisampleEnable = FALSE;
	rastDesc.AntialiasedLineEnable = FALSE;
	rastDesc.ForcedSampleCount = 0;
	rastDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	

	graphicsPipelineStateDesc.RasterizerState = rastDesc;

	D3D12_DEPTH_STENCIL_DESC depthDesc{};
	depthDesc.DepthEnable = TRUE;
	depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthDesc.StencilEnable = FALSE;
	depthDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_LESS;
	depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_LESS;

	depthDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

	depthDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;

	depthDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	   
	graphicsPipelineStateDesc.DepthStencilState = depthDesc;
	
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

void GeometryPass::_CreateViewPort()
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
void GeometryPass::_CopyLightData()
{
	STRUCTS::LIGHT_VALUES values;

	PointLight * pl;
	DirectionalLight * dl;

	struct
	{
		UINT a, b, c, d;
	} uint4;

	uint4.a = static_cast<UINT>(p_lightQueue.size());
	m_lightIndexBuffer.SetData(&uint4, sizeof(UINT) * 4);


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
		m_lightsArrayBuffer.SetData(&values, sizeof(STRUCTS::LIGHT_VALUES), i * sizeof(STRUCTS::LIGHT_VALUES));
	}
}


HRESULT GeometryPass::_Init()
{
	HRESULT hr = 0;

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
	
	_CreateViewPort();

	return hr;
}
