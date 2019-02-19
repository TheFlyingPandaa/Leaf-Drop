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
	static const UINT INCREMENT_LEVEL = 8; // DONT YOU DARE CHANGE THIS
	static const DirectX::XMFLOAT3 DIR[] = {
		{ 1.0f, 1.0f, 1.0f },
		{ 3.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 3.0f },
		{ 3.0f, 1.0f, 3.0f },
		{ 1.0f, 3.0f, 1.0f },
		{ 3.0f, 3.0f, 1.0f },
		{ 1.0f, 3.0f, 3.0f },
		{ 3.0f, 3.0f, 3.0f }
	};
	DirectX::XMVECTOR vecDir[8];
	for (UINT i = 0; i < INCREMENT_LEVEL; i++)
	{
		vecDir[i] = DirectX::XMLoadFloat3(&DIR[i]);
	}

	m_ocTree.clear();
	UINT totalAABB = 0;
	for (UINT level = 0; level < treeLevel; level++)
	{
		totalAABB += std::pow(INCREMENT_LEVEL, level + 1);
	}
	m_ocTree = std::vector<AABB>(totalAABB);

	float pos = -(int)worldSize / 2;
	DirectX::XMFLOAT3 startPos(pos, pos, pos);

	UINT counter = 0;
	UINT futureAABB = 0;
	for (UINT level = 0; level < treeLevel; level++)
	{
		float size = (worldSize / std::pow(2, level + 1)) / 2;

		DirectX::XMFLOAT3 axisSize(size, size, size);

		DirectX::XMVECTOR vecAxisSize = DirectX::XMLoadFloat3(&axisSize);
		DirectX::XMVECTOR vecStartPos = DirectX::XMLoadFloat3(&startPos);

		UINT targetDir = 0;
		UINT nrOfAABB = std::pow(INCREMENT_LEVEL, level + 1);
		UINT inc = 0;
		futureAABB += nrOfAABB;
		for (UINT i = 0; i < nrOfAABB; i++)
		{
			AABB aabb;
			aabb.axis = axisSize;
			aabb.level = level;
			aabb.nrOfSubAABB = INCREMENT_LEVEL;
			aabb.numberOfTriangles = 0;
			/*aabb.subAABBIndexStart = futureAABB;
			aabb.subAABBIndexEnd = futureAABB + INCREMENT_LEVEL;*/

			DirectX::XMStoreFloat3(&aabb.position,
				DirectX::XMVectorAdd(vecStartPos,
					DirectX::XMVectorMultiply(vecAxisSize,vecDir[targetDir++])));

			if (targetDir % 8 == 0 && i != 0)
			{
				targetDir = 0;
				vecStartPos = DirectX::XMVectorAdd(vecStartPos,
					DirectX::XMVectorMultiply(vecAxisSize, vecDir[inc++]));
				inc = inc % 8;
			}

			m_ocTree[counter++] = aabb;
		}
	}

	
	std::ofstream stream;
	stream.open("OctTree test.txt");

	UINT c = 0;
	for (auto & s : m_ocTree)
	{
		stream << "[" << c++ << "]\n";
		stream << s.ToString() << "\n";

	}
	stream.close();
	size_t triSize = triangles.size();
	for (size_t i = 0; i < triSize; i++)
	{

	}
}

const std::vector<AABB>& OcTree::GetTree() const
{
	return m_ocTree;
}
