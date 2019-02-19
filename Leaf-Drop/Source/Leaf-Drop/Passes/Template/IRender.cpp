#include "CorePCH.h"
#include "IRender.h"
#include "Source/Leaf-Drop/Objects/Drawable.h"
#include "../../Objects/Lights/Template/ILight.h"
void IRender::Clear()
{
	p_drawQueue.clear();
	p_lightQueue.clear();
}

void IRender::UpdateThread()
{
	if (m_threadDone && m_threadRunning && m_thread.get_id() != std::thread::id())
	{
		m_threadDone = false;
	}
	else
	{
		if (m_thread.get_id() == std::thread::id())
		{
			m_threadRunning = true;
			m_thread = std::thread(&IRender::_UpdateThread, this);
		}
		m_threadDone = false;
	}
}

#pragma optimize( "", off )
void IRender::ThreadJoin()
{
	while (!m_threadDone);
}
#pragma optimize( "", on )

void IRender::KillThread()
{
	m_threadRunning = false;
	if (m_thread.get_id() != std::thread::id())
	{
		m_thread.join();
	}
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

	m_threadDone = false;
	m_threadRunning = false;
}

void IRender::p_ReleaseCommandList()
{
	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		SAFE_RELEASE(p_commandList[i]);
		SAFE_RELEASE(p_commandAllocator[i]);
	}
}

void IRender::_UpdateThread()
{
	if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
	{
		PRINT("FAILED TO SET PRIORITY LEVEL OF THREAD \n");
	}
	while (m_threadRunning)
	{
		if (!m_threadDone)
		{
			this->Update();
			this->Draw();
			m_threadDone = true;
		}
	}
	m_threadRunning = false;
}

HRESULT IRender::p_CreateCommandList(const std::wstring & name, D3D12_COMMAND_LIST_TYPE listType)
{
	HRESULT hr = 0;
	D3D12_COMMAND_LIST_TYPE type{};
	

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		if (FAILED(hr = p_coreRender->GetDevice()->CreateCommandAllocator(
			listType,
			IID_PPV_ARGS(&p_commandAllocator[i]))))
		{
			return hr;
		}
		SET_NAME(p_commandAllocator[i], std::wstring(name + std::wstring(L" CommandAllocator") + std::to_wstring(i)).c_str());

		if (FAILED(hr = p_coreRender->GetDevice()->CreateCommandList(0, listType, p_commandAllocator[i], nullptr, IID_PPV_ARGS(&p_commandList[i]))))
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
	auto it = std::find(p_drawQueue.begin(), p_drawQueue.end(), drawable);

	if (it == p_drawQueue.end())
	{
		p_drawQueue.push_back(drawable);
	}
	else
	{
		it._Ptr->ObjectData.push_back(drawable);
	}
}

void IRender::SubmitLight(ILight * light)
{
	p_lightQueue.push_back(light);
}

IRender::InstanceGroup::InstanceGroup(Drawable * drawable)
{
	ObjectData.push_back(drawable);
	DiffuseTexture = drawable->GetTexture();
	NormalTexture = drawable->GetNormal();
	MetallicTexture = drawable->GetMetallic();
	MeshPtr = drawable->GetMesh();
}

bool IRender::InstanceGroup::operator==(Drawable * drawable)
{
	return	DiffuseTexture == drawable->GetTexture() &&
			NormalTexture == drawable->GetNormal() &&
			MetallicTexture == drawable->GetMetallic() &&
			MeshPtr == drawable->GetMesh();
}

IRender::InstanceGroup::ObjectData::ObjectData(Drawable * drawable)
{
	WorldMatrix = drawable->GetWorldMatrix();
}
