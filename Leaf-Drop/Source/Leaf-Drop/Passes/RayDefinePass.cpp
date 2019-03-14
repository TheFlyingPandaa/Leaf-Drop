#include "CorePCH.h"
#include "RayDefinePass.h"
#include "../Wrappers/ShaderCreator.h"

#define RAY_STENCIL				0
#define POSITION				1
#define NORMAL					2
#define METALLIC				3

RayDefinePass::RayDefinePass() : IRender()
{
	timer.OpenLog("raydefine.txt");
}


RayDefinePass::~RayDefinePass()
{
	timer.CloseLog();
}

HRESULT RayDefinePass::Init()
{
	return _Init();
}

void RayDefinePass::Update()
{
	//p_coreRender->GetFence(PRE_PASS)->Wait(p_coreRender->GetCommandQueue());
	OpenCommandList(m_pipelineState);

	const UINT frameIndex = p_coreRender->GetFrameIndex();
	ID3D12GraphicsCommandList * commandList = p_commandList[frameIndex];
	p_coreRender->SetResourceDescriptorHeap(commandList);
		
	commandList->SetGraphicsRootSignature(m_rootSignature);
	m_rayStencil.Bind(RAY_STENCIL, commandList);

	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	for (UINT i = 0; i < m_geometryRenderTargetsSize; i++)
	{
		m_geometryRenderTargets[i]->SetGraphicsRootDescriptorTable(i + 1, commandList);
	}
}

void RayDefinePass::Draw()
{
	const UINT frameIndex = p_coreRender->GetFrameIndex();
	ID3D12GraphicsCommandList * commandList = p_commandList[frameIndex];
	
	commandList->IASetVertexBuffers(0, 1, &m_screenQuad.vertexBufferView);
	commandList->DrawInstanced((UINT)m_screenQuad.mesh.size(), 1, 0, 0);

	ExecuteCommandList();
	
	p_coreRender->GetComputePass()->SetRayData(&m_rayStencil);

	p_coreRender->GetFence(DEFINE)->Signal(p_coreRender->GetCommandQueue());
}

void RayDefinePass::Release()
{
	m_screenQuad.Release();
	m_rayStencil.Release();
	
	SAFE_RELEASE(m_pipelineState);
	SAFE_RELEASE(m_rootSignature);
	p_ReleaseCommandList();
}

const UAV * RayDefinePass::GetRayStencil() const
{
	return &m_rayStencil;
}

void RayDefinePass::SetGeometryRenderTargets(RenderTarget * const * geometryRenderTargets, UINT arraySize)
{
	m_geometryRenderTargets = geometryRenderTargets;
	m_geometryRenderTargetsSize = arraySize;
}

HRESULT RayDefinePass::_Init()
{
	HRESULT hr = 0;

	if (FAILED(hr = p_CreateCommandList(L"DefRay")))
		return hr;

	const UINT FRAME_INDEX = p_coreRender->GetFrameIndex();
	ID3D12GraphicsCommandList * commandList = p_coreRender->GetCommandList()[FRAME_INDEX];
	if (FAILED(hr = _CreateScreenQuad(p_coreRender, FRAME_INDEX, commandList)))
	{
		return hr;
	}
	if (FAILED(hr = _InitRootSignature()))
		return hr;

	if (FAILED(hr = _InitShader()))
		return hr;

	if (FAILED(hr = _InitPipelineState()))
		return hr;

	_CreateViewPort();
	   
	struct RAY_STRUCT
	{
		DirectX::XMFLOAT3	startPos;
		DirectX::XMFLOAT3	normal;
		bool				dispatch;
	};

	Window * wnd = Window::GetInstance();
	POINT p = wnd->GetWindowSize();
	UINT elements = ((p.x / SCREEN_DIV) * (p.y / SCREEN_DIV));

	if (FAILED(hr = m_rayStencil.Init(L"RayStencil", elements * sizeof(RAY_STRUCT), elements, sizeof(RAY_STRUCT))))
	{
		return hr;
	}
	
	return hr;
}

HRESULT RayDefinePass::_CreateScreenQuad(CoreRender * coreRender, const UINT & frameIndex, ID3D12GraphicsCommandList * commandList)
{
	HRESULT hr = 0;

	// Create ScreenQuad
	ScreenQuad::Vertex topLeft(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	ScreenQuad::Vertex botLeft(-1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	ScreenQuad::Vertex topRight(1.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	ScreenQuad::Vertex botRight(1.0f, 1.0f, 0.0f, 1.0f, .0f);


	m_screenQuad.mesh.push_back(topLeft);
	m_screenQuad.mesh.push_back(botLeft);
	m_screenQuad.mesh.push_back(topRight);
	m_screenQuad.mesh.push_back(botRight);


	if (SUCCEEDED(coreRender->OpenCommandList()))
	{
		m_screenQuad.vertexBufferSize = static_cast<UINT>(sizeof(ScreenQuad::Vertex) * 4);

		if (SUCCEEDED(coreRender->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_screenQuad.vertexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_screenQuad.vertexBuffer))))
		{
			SET_NAME(m_screenQuad.vertexBuffer, std::wstring(std::wstring(L"Deferred :") +
				std::wstring(L": vertexBuffer")).c_str());

			if (SUCCEEDED(coreRender->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(m_screenQuad.vertexBufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_screenQuad.vertexUploadBuffer))))
			{
				SET_NAME(m_screenQuad.vertexUploadBuffer, std::wstring(std::wstring(L"Deferred :") +
					std::wstring(L": vertexUploadBuffer")).c_str());

				D3D12_SUBRESOURCE_DATA vertexData = {};
				vertexData.pData = reinterpret_cast<void*>(m_screenQuad.mesh.data());
				vertexData.RowPitch = m_screenQuad.vertexBufferSize;
				vertexData.SlicePitch = m_screenQuad.vertexBufferSize;

				UpdateSubresources(commandList, m_screenQuad.vertexBuffer, m_screenQuad.vertexUploadBuffer, 0, 0, 1, &vertexData);

				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_screenQuad.vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
				commandList->Close();
				if (SUCCEEDED(coreRender->ExecuteCommandList()))
				{
					m_screenQuad.vertexBufferView.BufferLocation = m_screenQuad.vertexBuffer->GetGPUVirtualAddress();
					m_screenQuad.vertexBufferView.StrideInBytes = sizeof(ScreenQuad::Vertex);
					m_screenQuad.vertexBufferView.SizeInBytes = m_screenQuad.vertexBufferSize;
					return true;
				}
			}
		}
	}

	return hr;
}

HRESULT RayDefinePass::_InitRootSignature()
{
	HRESULT hr = 0;

	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0,
		D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		16u,
		D3D12_COMPARISON_FUNC_NEVER,
		D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
		0.0f,
		D3D12_FLOAT32_MAX);

	CD3DX12_STATIC_SAMPLER_DESC compSamplerDesc = CD3DX12_STATIC_SAMPLER_DESC(1,
		D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		16u,
		D3D12_COMPARISON_FUNC_GREATER_EQUAL,
		D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
		0.0f,
		D3D12_FLOAT32_MAX);

	D3D12_STATIC_SAMPLER_DESC sampler[]{ samplerDesc, compSamplerDesc };

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;

	D3D12_DESCRIPTOR_RANGE1 descRange = CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);
	D3D12_DESCRIPTOR_RANGE1 descRange1 = CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 1);
	D3D12_DESCRIPTOR_RANGE1 descRange2 = CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 1);


	CD3DX12_ROOT_PARAMETER1 rootParameters[4];
	rootParameters[RAY_STENCIL].InitAsUnorderedAccessView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[POSITION].InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[NORMAL].InitAsDescriptorTable(1, &descRange1, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[METALLIC].InitAsDescriptorTable(1, &descRange2, D3D12_SHADER_VISIBILITY_PIXEL);

	rootSignatureDesc.Init_1_1(
		_countof(rootParameters),
		rootParameters,
		_countof(sampler),
		sampler,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
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
	else
	{
		
		OutputDebugStringA(static_cast<char*>(error->GetBufferPointer()));

		error->Release();
	}

	return hr;
}

HRESULT RayDefinePass::_InitShader()
{
	HRESULT hr = 0;


	ID3DBlob * blob = nullptr;
	if (FAILED(hr = ShaderCreator::CreateShader(L"..\\Leaf-Drop\\Source\\Leaf-Drop\\Shaders\\DeferredPass\\DefaultDeferredVertex.hlsl", blob, "vs_5_1")))
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
		NULL,NULL };

	if (FAILED(hr = ShaderCreator::CreateShader(L"..\\Leaf-Drop\\Source\\Leaf-Drop\\Shaders\\RayDefinePass\\DefaultRayDefine.hlsl", blob, "ps_5_1", def)))
	{
		return hr;
	}
	m_pixelShader.pShaderBytecode = blob->GetBufferPointer();
	m_pixelShader.BytecodeLength = blob->GetBufferSize();

	return hr;
}

HRESULT RayDefinePass::_InitPipelineState()
{
	HRESULT hr = 0;
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(DirectX::XMFLOAT4), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	m_inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	m_inputLayoutDesc.pInputElementDescs = inputLayout;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.InputLayout = m_inputLayoutDesc;
	graphicsPipelineStateDesc.pRootSignature = m_rootSignature;
	graphicsPipelineStateDesc.VS = m_vertexShader;
	graphicsPipelineStateDesc.PS = m_pixelShader;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsPipelineStateDesc.SampleMask = 0xffffffff;

	D3D12_RASTERIZER_DESC rastDesc{};
	rastDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rastDesc.CullMode = D3D12_CULL_MODE_BACK;
	rastDesc.FrontCounterClockwise = FALSE;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0.0f;
	rastDesc.SlopeScaledDepthBias = 0.0f;
	rastDesc.DepthClipEnable = FALSE;
	rastDesc.MultisampleEnable = FALSE;
	rastDesc.AntialiasedLineEnable = FALSE;
	rastDesc.ForcedSampleCount = 0;
	rastDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;


	graphicsPipelineStateDesc.RasterizerState = rastDesc;

	D3D12_DEPTH_STENCIL_DESC depthDesc{};
	depthDesc.DepthEnable = FALSE;
	depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthDesc.StencilEnable = FALSE;
	depthDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

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

void RayDefinePass::_CreateViewPort()
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
