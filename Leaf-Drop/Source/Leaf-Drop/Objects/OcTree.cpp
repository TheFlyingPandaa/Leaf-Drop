#include "CorePCH.h"
#include "OcTree.h"

#define INCREMENT_LEVEL 8


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
	m_ocTree.clear();
	m_ocTree = std::vector<AABB>(totalAABB);

	DirectX::XMFLOAT3 startPos(0, 0, 0);
	_BuildTree(startPos, 0, treeLevel, worldSize, 0);

	
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

void OcTree::_BuildTree(const DirectX::XMFLOAT3 & startPos, const UINT & level, const UINT & maxLevel, const UINT & worldSize, const UINT & currentStartIndex)
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
		//m_ocTree[currentStartIndex + i] = aabb;
		aabb.childrenIndexStart = m_ocTree.size() + 1;
		aabb.childrenIndexEnd = aabb.childrenIndexStart + INCREMENT_LEVEL;

		m_ocTree.push_back(aabb);

		
		if (level + 1< maxLevel)
			_BuildTree(aabb.position, level + 1, maxLevel, worldSize, currentStartIndex + nrOfAABB + i);
	}
	   	//startPos = m_ocTree[counter - 1].position;
	
}
