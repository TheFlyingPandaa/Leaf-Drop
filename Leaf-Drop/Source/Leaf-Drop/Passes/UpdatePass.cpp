#include "CorePCH.h"
#include "UpdatePass.h"

#include "../Wrappers/ShaderCreator.h"
#include "../Objects/Camera.h"
#include "../Objects/StaticMesh.h"

#include "Source/Leaf-Drop/Objects/Lights/PointLight.h"
#include "Source/Leaf-Drop/Objects/Lights/DirectionalLight.h"

#include "../Utillity/Timer.h"

#define BUILD_TREE -512, -512, -512, 3u, 1024u

UpdatePass::UpdatePass() : IRender()
{
}

UpdatePass::~UpdatePass()
{
}

HRESULT UpdatePass::Init()
{
	HRESULT hr = 0;
	m_staticOctree.BuildTree(BUILD_TREE);
	m_octree.BuildTree(BUILD_TREE);
	m_octree.CreateBuffer(L"Octree");

	if (FAILED(hr = m_objectData.Init(MAX_OBJECTS * sizeof(STRUCTS::ObjectValues), L"ObjectData", ConstantBuffer::STRUCTURED_BUFFER, sizeof(STRUCTS::ObjectValues))))
	{
		return hr;
	}
	if (FAILED(hr = m_octreeBuffer.Init(TO_BYTE(4), L"Octree buffer", ConstantBuffer::STRUCTURED_BUFFER, 1)))
	{
		return hr;
	}
	if (FAILED(hr = m_lightBuffer.Init(MAX_LIGHTS * sizeof(STRUCTS::LIGHT_VALUES), L"Lights", ConstantBuffer::STRUCTURED_BUFFER, sizeof(STRUCTS::LIGHT_VALUES))))
	{
		return hr;
	}
	if (FAILED(hr = m_fence.CreateFence(p_coreRender->GetCopyQueue())))
	{
		return hr;
	}

	return hr;
}

void UpdatePass::Update()
{
	static bool _FirstRun = true;

	if (_FirstRun)
	{
		_PlaceStaticTree();
		_FirstRun = false;
	}

	_PlaceDynamicTree();

	p_coreRender->BeginCopy();
	m_octree.WriteToBuffer(p_coreRender->GetCopyCommandList(), m_octreeBuffer.GetResource());
	p_coreRender->EndCopy();

	_SetLightData();
	_SetObjectData();
	m_fence.WaitForFinnishExecution();
}

void UpdatePass::Draw()
{

}

void UpdatePass::Release()
{
	m_octreeBuffer.Release();
	m_objectData.Release();
	m_lightBuffer.Release();
	m_octree.Release();
	m_fence.Release();
}

const ConstantBuffer & UpdatePass::GetLightBuffer() const
{
	return m_lightBuffer;
}

const UpdatePass::RayData & UpdatePass::GetRayData() const
{
	return m_rayData;
}

void UpdatePass::_PlaceStaticTree()
{
	for (UINT dq = 0; dq < p_staticDrawQueue.size(); dq++)
	{
		for (UINT m = 0; m < p_staticDrawQueue[dq].DrawableObjectData.size(); m++)
		{
			DirectX::XMFLOAT4X4A WorldInverse = p_staticDrawQueue[dq].DrawableObjectData[m].WorldMatrix;
			DirectX::XMStoreFloat4x4A(&WorldInverse, DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4A(&WorldInverse)))));
			auto aabb = p_staticDrawQueue[dq].MeshPtr->GetAABB();
			STRUCTS::ObjectValues ocv;
			ocv.World = p_staticDrawQueue[dq].DrawableObjectData[m].WorldMatrix;
			ocv.WorldInverse = WorldInverse;
			ocv.MeshIndex = aabb.meshIndex;
			ocv.Min = aabb.min;
			ocv.Max = aabb.max;
			ocv.TextureIndex = p_staticDrawQueue[dq].TextureOffset;
			m_staticOctreeObjects.push_back(ocv);
		}
	}

	m_staticOctree.PlaceObjects(m_staticOctreeObjects);

	m_offset = m_staticOctreeObjects.size() * sizeof(STRUCTS::ObjectValues);
	m_objectData.SetData(m_staticOctreeObjects.data(), m_staticOctreeObjects.size() * sizeof(STRUCTS::ObjectValues), 0,true);
}

void UpdatePass::_PlaceDynamicTree()
{
	m_dynamicOctreeObjects.clear();
	for (UINT dq = 0; dq < p_dynamicDrawQueue.size(); dq++)
	{
		for (UINT m = 0; m < p_dynamicDrawQueue[dq].DrawableObjectData.size(); m++)
		{
			DirectX::XMFLOAT4X4A WorldInverse = p_dynamicDrawQueue[dq].DrawableObjectData[m].WorldMatrix;
			DirectX::XMStoreFloat4x4A(&WorldInverse, DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4A(&WorldInverse)))));
			auto aabb = p_dynamicDrawQueue[dq].MeshPtr->GetAABB();
			STRUCTS::ObjectValues ocv;
			ocv.World = p_dynamicDrawQueue[dq].DrawableObjectData[m].WorldMatrix;
			ocv.WorldInverse = WorldInverse;
			ocv.MeshIndex = aabb.meshIndex;
			ocv.Min = aabb.min;
			ocv.Max = aabb.max;
			ocv.TextureIndex = p_dynamicDrawQueue[dq].TextureOffset;
			m_dynamicOctreeObjects.push_back(ocv);
		}
	}

	m_octree.PlaceObjects(m_dynamicOctreeObjects, true);
	m_octree.Merge(m_staticOctree);
	m_rayData.NumberOfObjectsInTree = (UINT)(m_staticOctreeObjects.size() + m_dynamicOctreeObjects.size());
}

void UpdatePass::_SetLightData()
{
	STRUCTS::LIGHT_VALUES values;

	PointLight * pl;
	DirectionalLight * dl;

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
		m_lightBuffer.SetData(&values, sizeof(STRUCTS::LIGHT_VALUES), i * sizeof(STRUCTS::LIGHT_VALUES));
	}
}

void UpdatePass::_SetObjectData()
{
	m_objectData.SetData(m_dynamicOctreeObjects.data(), m_dynamicOctreeObjects.size() * sizeof(STRUCTS::ObjectValues), m_offset, true);
}
