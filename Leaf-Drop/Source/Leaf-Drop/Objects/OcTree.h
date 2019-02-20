#pragma once
#include <vector>
#include <sal.h>

struct AABB
{
	~AABB()
	{
		if (triangleIndices)
			delete[] triangleIndices;
		triangleIndices = nullptr;
	}


	DirectX::XMFLOAT3	position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3	axis = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
	UINT				level = 0;
	
	UINT				nrOfChildren = 8;
	UINT				childrenIndices[8];

	UINT *				triangleIndices = nullptr;
	UINT				nrOfTriangles = 0;

	AABB & operator=(const AABB & other)
	{
		if (this != &other)
		{
			position = other.position;
			axis = other.axis;
			level = other.level;
			nrOfChildren = other.nrOfChildren;
			nrOfTriangles = other.nrOfTriangles;

			if (nrOfTriangles)
			{
				triangleIndices = new UINT[nrOfTriangles];
				for (UINT i = 0; i < nrOfTriangles; i++)
					triangleIndices[i] = other.triangleIndices[i];

			}

			for (UINT i = 0; i < nrOfChildren; i++)
				childrenIndices[i] = other.childrenIndices[i];
		}
		return *this;
	}

	std::string ToString()
	{
		std::string str = "";
		str += "Position: " + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + "\n";
		str += "Axis: " + std::to_string(axis.x) + ", " + std::to_string(axis.y) + ", " + std::to_string(axis.z) + "\n";
		str += "Level: " + std::to_string(level) + "\n";
		str += "NrOfSubAABB: " + std::to_string(nrOfSubAABB) + "\n";
		str += "TriangleStartIndex: " + std::to_string(triangleStartIndex) + "\n";
		str += "NumberOfTriangles: " + std::to_string(numberOfTriangles) + "\n";
		str += "ChildrenIndexStart: " + std::to_string(childrenIndexStart)+ "\n";
		str += "ChildrenIndexEnd: " + std::to_string(childrenIndexEnd)+ "\n";

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
	void _BuildTree(const DirectX::XMFLOAT3 & startPos, const UINT & level, const UINT & maxLevel, const UINT & worldSize, const UINT & currentStartIndex);
};

