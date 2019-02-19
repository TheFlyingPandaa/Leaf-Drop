#pragma once
#include <vector>
#include <sal.h>

struct AABB
{
	DirectX::XMFLOAT3	position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3	axis = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
	UINT				level = 0;
	UINT				nrOfSubAABB = 0;
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
			nrOfSubAABB = other.nrOfSubAABB;
			triangleStartIndex = other.triangleStartIndex;
			numberOfTriangles = other.numberOfTriangles;
			for (UINT i = 0; i < nrOfSubAABB; i++)
				subAABBIndexArr[i] = other.subAABBIndexArr[i];
		}
		return *this;
	}

	std::string ToString()
	{
		std::string str = "";
		str += "Position: " + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + "\n";
		str += "axis: " + std::to_string(axis.x) + ", " + std::to_string(axis.y) + ", " + std::to_string(axis.z) + "\n";
		str += "level: " + std::to_string(level) + "\n";
		str += "nrOfSubAABB: " + std::to_string(nrOfSubAABB) + "\n";
		str += "triangleStartIndex: " + std::to_string(triangleStartIndex) + "\n";
		str += "numberOfTriangles: " + std::to_string(numberOfTriangles) + "\n";
		str += "SubAABBIndex: ";
		for (UINT i = 0; i < nrOfSubAABB; i++)
			str += std::to_string(subAABBIndexArr[i]) += ", ";
		str += "\n";

		return str;
	}

};

class OcTree
{
public:
	OcTree();
	~OcTree();

	void BuildTree(std::vector<STRUCTS::Triangle> & triangles, UINT treeLevel = 3, UINT worldSize = 1024);
	const std::vector<AABB> & GetTree() const;

private:
	std::vector<AABB> m_ocTree;
};

