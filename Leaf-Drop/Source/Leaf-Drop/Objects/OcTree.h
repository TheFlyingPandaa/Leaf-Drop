#pragma once
#include <vector>
#include <sal.h>

struct AABB
{
	DirectX::XMFLOAT3	position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3	axis = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
	UINT				level = 0;
	UINT				nrOfSubAABBIndex = 0;
	UINT				subAABBIndexArr[8];
	UINT				triangleStartIndex = 0;
	UINT				numberOfTriangles = 0;

	AABB & operator=(const AABB & other)
	{
		if (this != &other)
		{
			position = other.position;
			axis = other.axis;
			level = other.level;
			nrOfSubAABBIndex = other.nrOfSubAABBIndex;
			triangleStartIndex = other.triangleStartIndex;
			numberOfTriangles = other.numberOfTriangles;
			for (UINT i = 0; i < nrOfSubAABBIndex; i++)
				subAABBIndexArr[i] = other.subAABBIndexArr[i];
		}
		return *this;
	}

};

class OcTree
{
public:
	OcTree();
	~OcTree();

	void BuildTree(std::vector<STRUCTS::Triangle> & triangles, UINT treeLevel = 3, UINT worldSize = 1000);
	const std::vector<AABB> & GetTree() const;

private:
	std::vector<AABB> m_ocTree;
};

