#include "CorePCH.h"
#include "OcTree.h"
#include <DirectXCollision.h>

OcTree::OcTree()
{
}


OcTree::~OcTree()
{
}

void OcTree::BuildTree(INT startX, INT startY, INT startZ, UINT treeLevel, UINT worldSize)
{
	using namespace DirectX;
	m_totalTreeByteSize = 0;
	m_leafCounter = 0;
	m_leafIndices.clear();
	m_ocTree.clear();
	m_maxLevel = treeLevel;

	size_t numberOfAABB = 0;
	size_t numberOfLeafs = (size_t)std::pow(INCREMENT_LEVEL, treeLevel);

	for (unsigned int level = 0; level <= treeLevel; level++)
		numberOfAABB += (size_t)std::pow(INCREMENT_LEVEL, level);

	XMFLOAT3 size((float)worldSize, (float)worldSize, (float)worldSize);
	XMFLOAT3 startPos((float)startX, (float)startY, (float)startZ);
	m_ocTree = std::vector<AABB>(numberOfAABB);
	m_leafIndices = std::vector<UINT>(numberOfLeafs);

	UINT index = 0;
	_CreateAABB(startPos, size, 0, treeLevel == 0, index++);

	UINT sizeOfNonLeaf = m_ocTree[0].byteSize;

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
					UINT parentIndex = _GetAABBIndexByWorldPos(currentPos, level - 1);
					m_ocTree[index].byteStart = m_ocTree[index].byteSize * index;
					m_ocTree[parentIndex].childrenIndices[m_ocTree[parentIndex].nrOfChildren] = index;
					m_ocTree[parentIndex].childrenByteAddress[m_ocTree[parentIndex].nrOfChildren++] = sizeOfNonLeaf * index++;
				}
			}
		}
	}
}

void OcTree::BuildTree(const DirectX::XMINT3 & startPos, UINT treeLevel, UINT worldSize)
{
	BuildTree(treeLevel, worldSize, startPos.x, startPos.y, startPos.z);
}

void OcTree::PlaceObjects(const std::vector<STRUCTS::MeshValues>& MeshValues, bool willRecieveAMerge)
{
	_clearLeafs();

	size_t meshValSize = MeshValues.size();

	for (UINT i = 0; i < meshValSize; i++)
		_traverseAndPlace(MeshValues[i], i, 0);


	if (!willRecieveAMerge)
	{
		size_t leafSize = m_leafCounter;

		UINT offset = m_ocTree[m_leafIndices[0]].byteStart;

		for (size_t j = 0; j < leafSize; j++)
		{
			size_t index = m_leafIndices[j];
			m_ocTree[index].CalcSize();
			m_ocTree[index].byteStart = offset;

			UINT parentIndex = _GetAABBIndexByWorldPos(m_ocTree[index].Min, m_ocTree[index].level - 1);

			if (m_ocTree[parentIndex].nrOfChildren == 8) m_ocTree[parentIndex].nrOfChildren = 0;

			m_ocTree[parentIndex].childrenByteAddress[m_ocTree[parentIndex].nrOfChildren++] = offset;

			offset += m_ocTree[index].byteSize;
		}
	}

	m_totalTreeByteSize = m_ocTree.back().byteStart + m_ocTree.back().byteSize;

}

void OcTree::Merge(const OcTree & other)
{
	size_t leafSize = m_leafCounter;

	UINT offset = m_ocTree[m_leafIndices[0]].byteStart;

	for (size_t j = 0; j < leafSize; j++)
	{
		size_t index = m_leafIndices[j];
		m_ocTree[index].meshDataIndices.insert(m_ocTree[index].meshDataIndices.end(), other.m_ocTree[index].meshDataIndices.begin(), other.m_ocTree[index].meshDataIndices.end());
		m_ocTree[index].nrOfObjects = m_ocTree[index].meshDataIndices.size();
		m_ocTree[index].CalcSize();
		m_ocTree[index].byteStart = offset;

		UINT parentIndex = _GetAABBIndexByWorldPos(m_ocTree[index].Min, m_ocTree[index].level - 1);
		if (m_ocTree[parentIndex].nrOfChildren == 8) m_ocTree[parentIndex].nrOfChildren = 0;
		m_ocTree[parentIndex].childrenByteAddress[m_ocTree[parentIndex].nrOfChildren++] = offset;

		offset += m_ocTree[index].byteSize;
	}

	m_totalTreeByteSize = m_ocTree.back().byteStart + m_ocTree.back().byteSize;
}

const UINT & OcTree::GetTotalTreeByteSize() const
{
	return m_totalTreeByteSize;
}

const std::vector<AABB>& OcTree::GetTree() const
{
	return m_ocTree;
}

std::string OcTree::ToString() const
{
	std::string str = "";

	str += "Max Level: " + std::to_string(m_maxLevel) + "\n";
	str += "Number Of Leafs: " + std::to_string(m_leafCounter) + "\n";
	str += "Total Byte Size: " + std::to_string(m_totalTreeByteSize) + "\n\n";

	UINT size = (UINT)m_ocTree.size();

	for (UINT i = 0; i < size; i++)
	{
		str += "[" + std::to_string(i) + "]\n";
		str += m_ocTree[i].ToString() + "\n";
	}

	return str;
}

bool OcTree::_inside(const AABB & aabb, const STRUCTS::MeshValues & colVal)
{
	using namespace DirectX;

	XMVECTOR colMin, colMax;
	XMVECTOR Min, Max;

	XMMATRIX worldInverse = XMMatrixTranspose(XMLoadFloat4x4A(&colVal.WorldInverse));

	colMin = XMLoadFloat3(&colVal.Min);
	colMax = XMLoadFloat3(&colVal.Max);
	Min = XMVector3TransformCoord(XMLoadFloat3(&aabb.Min), worldInverse);
	Max = XMVector3TransformCoord(XMLoadFloat3(&aabb.Max), worldInverse);

	BoundingBox a1, a2;

	BoundingBox::CreateFromPoints(a1, Min, Max);
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

	aabb.Min = position;
	aabb.Max = size;

	DirectX::XMStoreFloat3(&aabb.Max, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position), DirectX::XMLoadFloat3(&size)));

	aabb.level = level;
	aabb.CalcSize();

	m_ocTree[index] = aabb;

	if (isLeaf)
		m_leafIndices[m_leafCounter++] = index;
}

UINT OcTree::_GetAABBIndexByWorldPos(const DirectX::XMFLOAT3 & worldPos, UINT Level)
{
	using namespace DirectX;
	size_t levelStartIndex = 0;
	size_t levelEndIndex = 0;

	for (UINT i = 0; i < Level; i++)
		levelStartIndex += (UINT)std::pow(INCREMENT_LEVEL, i);

	levelEndIndex = levelStartIndex + (UINT)std::pow(INCREMENT_LEVEL, Level);

	XMFLOAT3 levelSize;

	XMStoreFloat3(&levelSize, XMVectorSubtract(XMLoadFloat3(&m_ocTree[levelStartIndex].Max), XMLoadFloat3(&m_ocTree[levelStartIndex].Min)));

	levelSize.x = 1.0f / levelSize.x;
	levelSize.y = 1.0f / levelSize.y;
	levelSize.z = 1.0f / levelSize.z;

	XMFLOAT3 masterNodePos = m_ocTree[0].Min;

	XMVECTOR worldStart = XMLoadFloat3(&masterNodePos);
	XMVECTOR searchPos = XMVectorSubtract(XMLoadFloat3(&worldPos), worldStart);
	XMFLOAT3 index;
	XMStoreFloat3(&index, XMVectorMultiply(searchPos, XMLoadFloat3(&levelSize)));

	UINT XYZ = (int)std::pow(2, Level);

	return (UINT)levelStartIndex + (UINT)index.x + XYZ * ((UINT)index.y + XYZ * (UINT)index.z);
}

void OcTree::_clearLeafs()
{
	for (UINT i = 0; i < m_leafCounter; i++)
	{
		UINT index = m_leafIndices[i];
		m_ocTree[index].meshDataIndices.clear();
		m_ocTree[index].nrOfObjects = 0;
	}
}

void OcTree::_traverseAndPlace(const STRUCTS::MeshValues & meshVal, UINT meshIndex, UINT ocIndex)
{
	if (_inside(m_ocTree[ocIndex], meshVal))
	{
		UINT nrOfChildren = 0;
		if ((nrOfChildren = m_ocTree[ocIndex].nrOfChildren) > 0)
		{
			for (UINT i = 0; i < nrOfChildren; i++)
				_traverseAndPlace(meshVal, meshIndex, m_ocTree[ocIndex].childrenIndices[i]);
		}
		else
		{
			m_ocTree[ocIndex].meshDataIndices.push_back(meshIndex);
			m_ocTree[ocIndex].nrOfObjects++;
		}
	}
}
