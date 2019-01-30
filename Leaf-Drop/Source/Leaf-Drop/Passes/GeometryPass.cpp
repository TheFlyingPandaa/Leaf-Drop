#include "CorePCH.h"
#include "GeometryPass.h"
#include "../Wrappers/ShaderCreator.h"
#include "../Objects/Drawable.h"
#include "../Objects/StaticMesh.h"
GeometryPass::GeometryPass() : IRender()
{
}


GeometryPass::~GeometryPass()
{
}

HRESULT GeometryPass::Init()
{
	HRESULT hr = 0;
	//if (FAILED(hr = p_CreateCommandList(L"Geometry")))
	//{
	//	return hr;
	//}
	//
	//if (FAILED(hr = OpenCommandList()))
	//{
	//	return hr;
	//}

	if (FAILED(hr = _Init()))
	{
		return hr;
	}

	if (FAILED(hr = m_camBuffer.Init(sizeof(DirectX::XMFLOAT4X4))))
	{
		return hr;
	}

	


	return hr;
}

void GeometryPass::Update()
{
	//if (FAILED(OpenCommandList()))
	//{
	//	return;
	//}

	const UINT frameIndex = p_coreRender->GetFrameIndex();
	ID3D12GraphicsCommandList * commandList = p_coreRender->GetCommandList()[frameIndex];
	
	commandList->SetPipelineState(m_pipelineState);
	commandList->SetGraphicsRootSignature(m_rootSignature);
	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

void GeometryPass::Draw()
{
	const UINT frameIndex = p_coreRender->GetFrameIndex();
	ID3D12GraphicsCommandList * commandList = p_coreRender->GetCommandList()[frameIndex];

	for (size_t i = 0; i < p_drawQueue.size(); i++)
	{
		commandList->IASetVertexBuffers(0, 1, &p_drawQueue[i]->GetMesh()->GetVBV());
		commandList->DrawInstanced(p_drawQueue[i]->GetMesh()->GetMesh()->size(), 1, 0, 0);
	}
	
	//ExecuteCommandList();
}

void GeometryPass::Release()
{
	//p_ReleaseCommandList();
	//SAFE_DELETE(m_renderTarget);
	SAFE_RELEASE(m_rootSignature);
	SAFE_RELEASE(m_pipelineState);
}

HRESULT GeometryPass::_InitRootSignature()
{
	HRESULT hr = 0;
		
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = 0;
	rootSignatureDesc.pParameters = nullptr;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;

	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	ID3DBlob * signature = nullptr;
	if (SUCCEEDED(hr = D3D12SerializeRootSignature(&rootSignatureDesc,
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

	if (FAILED(hr = ShaderCreator::CreateShader(L"..\\Leaf-Drop\\Source\\Leaf-Drop\\Shaders\\GeometryPass\\DefaultGeometryPixel.hlsl", blob, "ps_5_1")))
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
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 64, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
	UINT2 wndSize = p_window->GetWindowSize();
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = wndSize.x;
	m_viewport.Height = wndSize.y;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = wndSize.x;
	m_scissorRect.bottom = wndSize.y;
}

HRESULT GeometryPass::_Init()
{
	HRESULT hr = 0;

	//m_renderTarget = new RenderTarget();
	//if (FAILED(hr = m_renderTarget->Init(L"Geometry")))
	//{
	//	return hr;
	//}

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
