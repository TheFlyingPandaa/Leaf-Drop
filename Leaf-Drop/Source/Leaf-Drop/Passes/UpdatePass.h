#pragma once
#include "Template/IRender.h"
#include <vector>

class UpdatePass :	public IRender
{
public:
	struct RayData
	{
		const ConstantBuffer * OctreeBuffer;
		const ConstantBuffer * ObjectData;
		const ConstantBuffer * LightBuffer;
		UINT NumberOfObjectsInTree;
	};


public:
	UpdatePass();
	~UpdatePass();

	// Inherited via IRender
	virtual HRESULT Init() override;
	virtual void Update() override;
	virtual void Draw() override;
	virtual void Release() override;


	const ConstantBuffer & GetLightBuffer() const;
	const RayData & GetRayData() const;


private:
	OcTree m_staticOctree;
	OcTree m_octree;
	
	std::vector<STRUCTS::ObjectValues> m_staticOctreeObjects;
	std::vector<STRUCTS::ObjectValues> m_dynamicOctreeObjects;

	ConstantBuffer	m_octreeBuffer;
	ConstantBuffer	m_objectData;
	ConstantBuffer	m_lightBuffer;

	RayData m_rayData = { &m_octreeBuffer, &m_objectData, &m_lightBuffer };

	UINT m_offset = 0;

	Fence m_fence;

private:
	void _PlaceStaticTree();
	void _PlaceDynamicTree();
	void _SetLightData();
	void _SetObjectData();
};

