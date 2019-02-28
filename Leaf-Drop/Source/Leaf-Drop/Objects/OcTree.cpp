#include "CorePCH.h"
#include "OcTree.h"


OcTree::OcTree()
{
}


OcTree::~OcTree()
{
}

void OcTree::BuildTree(const std::vector<STRUCTS::OctreeValues>& ocVal, UINT treeLevel, UINT worldSize, UINT startX, UINT startY, UINT startZ)
{
	UINT totalAABB = 0;
	for (UINT level = 0; level < treeLevel; level++)
	{
		totalAABB += (UINT)std::pow(INCREMENT_LEVEL, level + 1);
	}
	totalAABB++;
	m_totalTreeByteSize = 0;
	m_leafIndices.clear();
	m_ocTree.clear();
	//m_ocTree = std::vector<AABB>(totalAABB);
	float size = (float)(worldSize / 2);
	DirectX::XMFLOAT3 startPos(startX, startY, startZ);
	DirectX::XMFLOAT3 axis(size, size, size);

	AABB startNode;
	startNode.position = startPos;
	startNode.axis = axis;
	startNode.nrOfChildren = INCREMENT_LEVEL;
	startNode.CalcSize();

	m_ocTree.push_back(startNode);

	_BuildTree(startPos, 0, treeLevel, worldSize, 0);

	size_t triSize = ocVal.size();
	size_t leafSize = m_leafIndices.size();
	for (size_t j = 0; j < leafSize; j++)
	{
		size_t index = m_leafIndices[j];
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

void OcTree::BuildTree2(const std::vector<STRUCTS::OctreeValues>& ocVal, UINT treeLevel, UINT worldSize, UINT startX, UINT startY, UINT startZ)
{
	using namespace DirectX;
	m_totalTreeByteSize = 0;
	m_leafIndices.clear();
	m_ocTree.clear();

	size_t numberOfAABB = 0;
	size_t numberOfLeafs = (size_t)std::pow(INCREMENT_LEVEL, treeLevel);

	for (unsigned int level = 0; level <= treeLevel; level++)
		numberOfAABB += (size_t)std::pow(INCREMENT_LEVEL, level);

	XMFLOAT3 size(worldSize, worldSize, worldSize);
	XMFLOAT3 startPos(startX, startY, startZ);
	m_ocTree = std::vector<AABB>(numberOfAABB);

	UINT index = 0;
	_CreateAABB(startPos, size, 0, treeLevel == 0, index++);

	for (UINT level = 1; level <= treeLevel; level++)
	{
		bool isLeaf = level == treeLevel;
		XMStoreFloat3(&size, XMVectorScale(XMLoadFloat3(&size), 0.5f));

		UINT XYZ = (UINT)std::pow(2, level);

		for (UINT z = 0; z < XYZ; z++)
		{
			XMFLOAT3 currentPos;
			currentPos.z = startPos.z + size.z * z;
			for (UINT y = 0; y < XYZ; y++)
			{
				currentPos.y = startPos.y + size.y * y;
				for (UINT x = 0; x < XYZ; x++)
				{
					currentPos.x = startPos.x + size.x * x;
					_CreateAABB(currentPos, size, level, isLeaf, index);
					UINT parentIndex = _GetParentIndex(currentPos, level - 1, treeLevel);
					m_ocTree[index].byteStart = m_ocTree[parentIndex].byteStart + m_ocTree[parentIndex].byteSize;
					m_ocTree[parentIndex].childrenIndices[m_ocTree[parentIndex].nrOfChildren++] = index++;
				}
			}
		}
	}

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
			m_leafIndices.push_back(m_ocTree.size() - 1);
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

void OcTree::_CreateAABB(const DirectX::XMFLOAT3 & position, const DirectX::XMFLOAT3 & size, UINT level, bool isLeaf, UINT index)
{
	AABB aabb;

	DirectX::XMVECTOR vecPos = DirectX::XMLoadFloat3(&position);
	DirectX::XMVECTOR vecSize = DirectX::XMLoadFloat3(&size);
	DirectX::XMVECTOR axis = DirectX::XMVectorScale(vecSize, 0.5f);

	DirectX::XMStoreFloat3(&aabb.axis, axis);
	DirectX::XMStoreFloat3(&aabb.position, DirectX::XMVectorAdd(vecPos, axis));
	aabb.level = level;
	aabb.CalcSize();

	m_ocTree[index] = aabb;

	if (isLeaf)
		m_leafIndices[m_leafCounter++] = index;
}

UINT OcTree::_GetParentIndex(const DirectX::XMFLOAT3 & worldPos, UINT currentLevel, UINT maxLevel)
{
	using namespace DirectX;
	size_t levelStartIndex = 0;
	size_t levelEndIndex = 0;

	for (UINT i = 0; i < currentLevel; i++)
		levelStartIndex += (UINT)std::pow(INCREMENT_LEVEL, i);

	levelEndIndex = levelStartIndex + (UINT)std::pow(INCREMENT_LEVEL, currentLevel);

	XMFLOAT3 levelSize = m_ocTree[levelStartIndex].axis;
	levelSize.x = 1.0f / levelSize.x;
	levelSize.y = 1.0f / levelSize.y;
	levelSize.z = 1.0f / levelSize.z;

	XMFLOAT3 masterNodePos = m_ocTree[0].position;
	XMFLOAT3 masterNodeSize = m_ocTree[0].axis;

	XMVECTOR worldStart = XMVectorSubtract(XMLoadFloat3(&masterNodeSize), XMLoadFloat3(&masterNodePos));
	XMVECTOR searchPos = XMVectorSubtract(XMLoadFloat3(&worldPos), worldStart);
	XMFLOAT3 index;
	XMStoreFloat3(&index, XMVectorMultiply(searchPos, XMLoadFloat3(&levelSize)));

	UINT XYZ = (int)std::pow(2, currentLevel);

	return (UINT)levelStartIndex + (UINT)index.x + XYZ * ((UINT)index.y + XYZ * (UINT)index.z);
}
