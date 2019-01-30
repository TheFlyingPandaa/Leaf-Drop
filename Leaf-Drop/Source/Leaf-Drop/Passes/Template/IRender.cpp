#include "CorePCH.h"
#include "IRender.h"

void IRender::Clear()
{
	p_drawQueue.clear();
}

HRESULT IRender::OpenCommandList(ID3D12PipelineState * pipelineSate)
{
	HRESULT hr = 0;
	
	const UINT frameIndex = p_coreRender->GetFrameIndex();

	if (SUCCEEDED(p_commandAllocator[frameIndex]->Reset()))
	{
		if (SUCCEEDED(p_commandList[frameIndex]->Reset(p_commandAllocator[frameIndex], pipelineSate)))
		{
			return hr;
		}
	}	
	return hr;
}

HRESULT IRender::ExecuteCommandList()
{
	HRESULT hr = 0;
	const UINT frameIndex = p_coreRender->GetFrameIndex();

	if (SUCCEEDED(hr = p_commandList[frameIndex]->Close()))
	{
		ID3D12CommandList* ppCommandLists[] = { p_commandList[frameIndex] };
		p_coreRender->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}
	return hr;
}

IRender::IRender()
{
	p_coreRender = CoreRender::GetInstance();
	p_window = Window::GetInstance();
}

void IRender::p_ReleaseCommandList()
{
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(p_commandList[i]);
		SAFE_RELEASE(p_commandAllocator[i]);
	}
}

HRESULT IRender::p_CreateCommandList(const std::wstring & name)
{
	HRESULT hr = 0;
	D3D12_COMMAND_LIST_TYPE type{};
	

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = p_coreRender->GetDevice()->CreateCommandAllocator(
			type, 
			IID_PPV_ARGS(&p_commandAllocator[i]))))
		{
			return hr;
		}
			SET_NAME(p_commandAllocator[i], std::wstring(name + std::wstring(L" CommandAllocator") + std::to_wstring(i)).c_str());

		if (FAILED(hr = p_coreRender->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, p_commandAllocator[i], nullptr, IID_PPV_ARGS(&p_commandList[i]))))
		{
			return hr;
		}
		SET_NAME(p_commandList[i], std::wstring(name + std::wstring(L" CommandList") + std::to_wstring(i)).c_str());

		p_commandList[i]->Close();
	}
	return hr;
}


IRender::~IRender()
{
}

void IRender::Submit(Drawable * drawable)
{
	p_drawQueue.push_back(drawable);
}

void IRender::SubmitLight(Light * light)
{
}