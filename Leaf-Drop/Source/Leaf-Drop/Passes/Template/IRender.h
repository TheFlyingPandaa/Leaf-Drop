#pragma once
//#include <EASTL/vector.h>
#include <vector>


class CoreRender;
class Window;
class Drawable;
class ILight;
class Texture;
class StaticMesh;

class IRender
{
public:
	virtual ~IRender();
	
	void Submit(Drawable * drawable);
	void SubmitLight(ILight * light);
		
	virtual HRESULT Init() = 0;
	virtual void Update() = 0; 
	virtual void Draw() = 0;
	virtual void Release() = 0;
	void Clear();

	void UpdateThread();
	void ThreadJoin();
	void KillThread();

	HRESULT OpenCommandList(ID3D12PipelineState * pipelineSate = nullptr);
	HRESULT ExecuteCommandList();
protected:
	IRender();

	struct InstanceGroup
	{
	public:
		struct ObjectDataStruct
		{
		public:
			ObjectDataStruct(Drawable * drawable);

			DirectX::XMFLOAT4X4A WorldMatrix;
			DirectX::XMFLOAT4A	Color;

		};
		
		std::vector<ObjectDataStruct> DrawableObjectData;
		Texture * DiffuseTexture;
		Texture * NormalTexture;
		Texture * MetallicTexture;
		StaticMesh * MeshPtr;

		InstanceGroup(Drawable * drawable);
		bool operator==(Drawable * drawable);
	};

	std::vector<InstanceGroup> p_drawQueue;
	std::vector<ILight*> p_lightQueue;
	CoreRender * p_coreRender = nullptr;
	Window * p_window = nullptr;

	ID3D12GraphicsCommandList * p_commandList[FRAME_BUFFER_COUNT]{ nullptr };
	ID3D12CommandAllocator * p_commandAllocator[FRAME_BUFFER_COUNT]{nullptr};
protected:
	
	HRESULT p_CreateCommandList(const std::wstring & name, D3D12_COMMAND_LIST_TYPE listType = D3D12_COMMAND_LIST_TYPE_DIRECT);
	void p_ReleaseCommandList();
private:
	bool m_threadRunning;
	bool m_threadDone;
	std::thread m_thread;
private:
	void _UpdateThread();

};

