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
	static const UINT REC_SIZE = 8;
	static const DirectX::XMFLOAT3 dir[] = {
		{ 1.0f, 1.0f, 1.0f },
		{ 2.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 2.0f },
		{ 2.0f, 1.0f, 2.0f },
		{ 1.0f, 2.0f, 1.0f },
		{ 2.0f, 2.0f, 1.0f },
		{ 1.0f, 2.0f, 2.0f },
		{ 2.0f, 2.0f, 2.0f },
	};

	m_ocTree.clear();
	UINT totalSize = REC_SIZE;
	for (UINT i = 0; i < treeLevel; i++)
		totalSize *= REC_SIZE;

	m_ocTree = std::vector<AABB>(totalSize);
	UINT counter = 0;

	DirectX::XMFLOAT3 startPos;
	startPos.x = -(int)worldSize / 2;
	startPos.y = -(int)worldSize / 2;
	startPos.z = -(int)worldSize / 2;

	DirectX::XMVECTOR vecStartPos = DirectX::XMLoadFloat3(&startPos);

	for (UINT level = 0; level < treeLevel; level++)
	{
		DirectX::XMFLOAT3 currentSize;
		currentSize.x = worldSize / ((level + 1) * REC_SIZE);
		currentSize.y = currentSize.x;
		currentSize.z = currentSize.x;
		DirectX::XMVECTOR vecCurrentSize = DirectX::XMLoadFloat3(&currentSize);

		for (UINT nrOfAABB = 0; nrOfAABB < std::pow(REC_SIZE, (level + 1)); nrOfAABB++)
		{
			AABB currentAABB;
			DirectX::XMVECTOR vecDir = DirectX::XMLoadFloat3(&dir[nrOfAABB % 8]);

			currentAABB.nrOfSubAABBIndex = REC_SIZE;
			currentAABB.level = level;
			currentAABB.numberOfTriangles = 0;
			currentAABB.axis = currentSize;

			DirectX::XMStoreFloat3(&currentAABB.position, DirectX::XMVectorAdd(vecStartPos, DirectX::XMVectorMultiply(vecCurrentSize, vecDir)));

			for (UINT subAABB = 0; subAABB < REC_SIZE; subAABB++)
			{
				currentAABB.subAABBIndexArr[subAABB] = std::pow(REC_SIZE, (level + 1));
			}

			m_ocTree[counter++] = currentAABB;

		}
	}
	
	size_t triSize = triangles.size();
	for (size_t i = 0; i < triSize; i++)
	{

	}
}

const std::vector<AABB>& OcTree::GetTree() const
{
	return m_ocTree;
}
