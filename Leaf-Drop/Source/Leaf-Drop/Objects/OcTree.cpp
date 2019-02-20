#include "CorePCH.h"
#include "OcTree.h"



OcTree::OcTree()
{
}


OcTree::~OcTree()
{
}

void OcTree::BuildTree(std::vector<STRUCTS::Triangle>& triangles, UINT treeLevel, UINT worldSize)
{
	UINT totalAABB = 0;
	for (UINT level = 0; level < treeLevel; level++)
	{
		totalAABB += std::pow(INCREMENT_LEVEL, level + 1);
	}
	totalAABB++;
	m_totalTreeByteSize = 0;
	m_leafIndex.clear();
	m_ocTree.clear();
	//m_ocTree = std::vector<AABB>(totalAABB);
	float size = worldSize / 2;
	DirectX::XMFLOAT3 startPos(0, 0, 0);
	DirectX::XMFLOAT3 axis(size, size, size);

	AABB startNode;
	startNode.position = startPos;
	startNode.axis = axis;
	startNode.nrOfChildren = INCREMENT_LEVEL;
	startNode.CalcSize();

	m_ocTree.push_back(startNode);

	_BuildTree(startPos, 0, treeLevel, worldSize, 0);

	size_t triSize = triangles.size();
	size_t leafSize = m_leafIndex.size();
	for (size_t j = 0; j < leafSize; j++)
	{
		size_t index = m_leafIndex[j];
		for (size_t i = 0; i < triSize; i++)
		{
			if (_inside(m_ocTree[index], triangles[i]))
			{
				m_ocTree[index].triangleIndices.push_back(i);
			}
		}
		m_ocTree[index].CalcSize();
	}

	size_t nrOf = m_ocTree.size();
	for (size_t i = 0; i < nrOf; i++)
	{
		m_totalTreeByteSize += m_ocTree[i].byteSize;
	}

	/*std::ofstream stream;
	stream.open("OcTree test.txt");

	UINT c = 0;
	for (auto & s : m_ocTree)
	{
		stream << "[" << c++ << "]\n";
		stream << s.ToString() << "\n";

	}

	stream.close();*/

	int breakMe = 0;
}

const UINT & OcTree::GetTotalTreeByteSize() const
{
	return m_totalTreeByteSize;
}

const std::vector<AABB>& OcTree::GetTree() const
{
	return m_ocTree;
}

void OcTree::_BuildTree(const DirectX::XMFLOAT3 & startPos, const UINT & level, const UINT & maxLevel, const UINT & worldSize, const size_t & parentIndex)
{
	using namespace DirectX;

	static const XMFLOAT3 DIR[] = {
		{-1.0f, -1.0f, -1.0f },
		{-1.0f, -1.0f,  1.0f },
		{ 1.0f, -1.0f,  1.0f },
		{ 1.0f, -1.0f, -1.0f },

		{-1.0f,  1.0f, -1.0f },
		{-1.0f,  1.0f,  1.0f },
		{ 1.0f,  1.0f,  1.0f },
		{ 1.0f,  1.0f, -1.0f }
	};

	UINT counter = 0;
	UINT futureAABB = 0;

	float size = (worldSize / std::pow(2, level + 1)) / 2;
	XMFLOAT3 axisSize(size, size, size);


	UINT nrOfAABB = std::pow(INCREMENT_LEVEL, level + 1);

	for (UINT i = 0; i < INCREMENT_LEVEL; i++)
	{
		XMVECTOR vecDir = XMLoadFloat3(&DIR[i]);

		AABB aabb;
		aabb.axis = axisSize;
		aabb.level = level;
		

		XMVECTOR moveMe = XMLoadFloat3(&startPos);
		XMVECTOR vecAxis = XMLoadFloat3(&axisSize);

		XMStoreFloat3(&aabb.position, XMVectorAdd(moveMe, (XMVectorScale(vecDir, size))));
		
		m_ocTree[parentIndex].childrenIndices[i] = m_ocTree.size();

		aabb.CalcSize();

		m_ocTree.push_back(aabb);
		
		if (level + 1 < maxLevel)
		{
			AABB & target = m_ocTree.back();
			target.nrOfChildren = INCREMENT_LEVEL;
			
			_BuildTree(aabb.position, level + 1, maxLevel, worldSize, m_ocTree.size() - 1);
		}
		else
			m_leafIndex.push_back(m_ocTree.size() - 1);
	}	
}

bool OcTree::_inside(const AABB & aabb, const STRUCTS::Triangle & tri)
{
	using namespace DirectX;
	XMFLOAT3 min, max, aabbPos, axis;
	XMFLOAT3 point[3];
	const STRUCTS::Vertex * v[3] = { &tri.v1, &tri.v2, &tri.v3 };

	for (UINT i = 0; i < 3; i++)
		memcpy(&point[i], v[i], sizeof(float) * 3);
	
	aabbPos		= aabb.position;
	axis		= aabb.axis;

	min.x = aabbPos.x - axis.x;
	min.y = aabbPos.y - axis.y;
	min.z = aabbPos.z - axis.z;

	max.x = aabbPos.x + axis.x;
	max.y = aabbPos.y + axis.y;
	max.z = aabbPos.z + axis.z;

	bool inside = false;
	for (int i = 0; i < 3 && !inside; i++)
	{
		inside = _pointInside(min, max, point[i]);
	}


	return inside;
}

bool OcTree::_pointInside(const DirectX::XMFLOAT3 & min, const DirectX::XMFLOAT3 & max, const DirectX::XMFLOAT3 & point)
{
	return	point.x > min.x && point.x < max.x &&
			point.y > min.y && point.y < max.y &&
			point.z > min.z && point.z < max.z;
}
