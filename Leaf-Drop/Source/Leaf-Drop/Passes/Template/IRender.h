#pragma once
//#include <EASTL/vector.h>
#include <vector>


class CoreRender;
class Window;
class Drawable;
class Light;
class Texture;
class StaticMesh;

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
	void Clear();

	HRESULT OpenCommandList(ID3D12PipelineState * pipelineSate = nullptr);
	HRESULT ExecuteCommandList();
protected:
	IRender();

	struct InstanceGroup
	{

		struct ObjectData
		{
			ObjectData(Drawable * drawable);

			DirectX::XMFLOAT4X4 WorldMatrix;
		};
		
		std::vector<ObjectData> ObjectData;
		Texture * TexPtr;
		StaticMesh * MeshPtr;

		InstanceGroup(Drawable * drawable);
		bool operator==(Drawable * drawable);
	};

	std::vector<InstanceGroup> p_drawQueue;
	CoreRender * p_coreRender = nullptr;
	Window * p_window = nullptr;

	ID3D12GraphicsCommandList * p_commandList[FRAME_BUFFER_COUNT]{ nullptr };
	ID3D12CommandAllocator * p_commandAllocator[FRAME_BUFFER_COUNT]{nullptr};
protected:
	
	HRESULT p_CreateCommandList(const std::wstring & name);
	void p_ReleaseCommandList();
	
};

