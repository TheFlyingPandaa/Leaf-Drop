#include "CorePCH.h"
#include "OcTree.h"


OcTree::OcTree()
{
}


OcTree::~OcTree()
{
}

void OcTree::BuildTree(const std::vector<STRUCTS::OctreeValues>& ocVal, UINT treeLevel, UINT worldSize)
{
	UINT totalAABB = 0;
	for (UINT level = 0; level < treeLevel; level++)
	{
		totalAABB += (UINT)std::pow(INCREMENT_LEVEL, level + 1);
	}
	totalAABB++;
	m_totalTreeByteSize = 0;
	m_leafIndex.clear();
	m_ocTree.clear();
	//m_ocTree = std::vector<AABB>(totalAABB);
	float size = (float)(worldSize / 2);
	DirectX::XMFLOAT3 startPos(0, 0, 0);
	DirectX::XMFLOAT3 axis(size, size, size);

	AABB startNode;
	startNode.position = startPos;
	startNode.axis = axis;
	startNode.nrOfChildren = INCREMENT_LEVEL;
	startNode.CalcSize();

	m_ocTree.push_back(startNode);

	_BuildTree(startPos, 0, treeLevel, worldSize, 0);

	size_t triSize = ocVal.size();
	size_t leafSize = m_leafIndex.size();
	for (size_t j = 0; j < leafSize; j++)
	{
		size_t index = m_leafIndex[j];
		for (size_t i = 0; i < triSize; i++)
		{
			if (_inside(m_ocTree[index], ocVal[i]))
			{
				m_ocTree[index].meshDataIndices.push_back((UINT)i);
			}
		}
		m_ocTree[index].nrOfObjects = (UINT)m_ocTree[index].meshDataIndices.size();
		m_ocTree[index].CalcSize();
	}

	size_t nrOf = m_ocTree.size();
	for (size_t i = 0; i < nrOf; i++)
	{
		if (i > 0)
			m_ocTree[i].byteStart = m_ocTree[i - 1].byteStart + m_ocTree[i - 1].byteSize;
		for (size_t j = 0; j < m_ocTree[i].nrOfChildren; j++)
		{
			UINT offset = 0;
			for (size_t k = m_ocTree[i].childrenIndices[j] - 1; k > i; k--)
			{
				offset += m_ocTree[k].byteSize;
			}
			m_ocTree[i].childrenByteAddress[j] = offset + m_ocTree[i].byteStart + m_ocTree[i].byteSize;
		}
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

	float size = (worldSize / (float)std::pow(2, level + 1)) / 2;
	XMFLOAT3 axisSize(size, size, size);


	UINT nrOfAABB = (UINT)std::pow(INCREMENT_LEVEL, level + 1);

	for (UINT i = 0; i < INCREMENT_LEVEL; i++)
	{
		XMVECTOR vecDir = XMLoadFloat3(&DIR[i]);

		AABB aabb;
		aabb.axis = axisSize;
		aabb.level = level;
		

		XMVECTOR moveMe = XMLoadFloat3(&startPos);
		XMVECTOR vecAxis = XMLoadFloat3(&axisSize);

		XMStoreFloat3(&aabb.position, XMVectorAdd(moveMe, (XMVectorScale(vecDir, size))));
		
		m_ocTree[parentIndex].childrenIndices[i] = (UINT)m_ocTree.size();

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

#include <DirectXCollision.h>

bool OcTree::_inside(const AABB & aabb, const STRUCTS::OctreeValues & colVal)
{
	using namespace DirectX;
	
	XMVECTOR colMin, colMax;

	colMin = XMLoadFloat3(&colVal.Min);
	colMax = XMLoadFloat3(&colVal.Max);

	// Turn aabb into localspace of Colval
	XMMATRIX wInv = XMMatrixTranspose(XMLoadFloat4x4A(&colVal.WorldInverse));
	XMVECTOR aabbPos = XMVector3TransformCoord(XMLoadFloat3(&aabb.position), wInv);
	XMVECTOR aabbAxis = XMVector3TransformCoord(XMLoadFloat3(&aabb.axis), wInv);
	
	BoundingBox a1, a2;
	
	XMStoreFloat3(&a1.Center, aabbPos);
	XMStoreFloat3(&a1.Extents, aabbAxis);

	BoundingBox::CreateFromPoints(a2, colMin, colMax);

	return a1.Intersects(a2);
}

bool OcTree::_pointInside(const DirectX::XMFLOAT3 & min, const DirectX::XMFLOAT3 & max, const DirectX::XMFLOAT3 & point)
{
	return	point.x > min.x && point.x < max.x &&
			point.y > min.y && point.y < max.y &&
			point.z > min.z && point.z < max.z;
}
