#pragma once
#include <EASTL/vector.h>
class CoreRender;
class Window;
class Drawable;
class Light;

class IRender
{
public:
	virtual ~IRender();
	
	void Submit(Drawable * drawable);
	void SubmitLight(Light * light);
		
	virtual HRESULT Init() = 0;
	virtual void Update() = 0; 
	virtual void Draw() = 0;
	virtual void Release() = 0;

	HRESULT OpenCommandList(ID3D12PipelineState * pipelineSate = nullptr);
	HRESULT ExecuteCommandList();
protected:
	IRender();

	CoreRender * p_coreRender = nullptr;
	Window * p_window = nullptr;

protected:
	ID3D12GraphicsCommandList * p_commandList[FRAME_BUFFER_COUNT]{ nullptr };
	ID3D12CommandAllocator * p_commandAllocator[FRAME_BUFFER_COUNT]{nullptr};
	
	HRESULT p_CreateCommandList();
	eastl::vector<Drawable*> p_drawQueue;
	
};

