#pragma once
#include <vector>
#include <sal.h>

#define INCREMENT_LEVEL 8

struct AABB
{
#pragma region stuff
	AABB() {};
	AABB(const AABB & other)
	{
		position = other.position;
		axis = other.axis;
		level = other.level;
		nrOfChildren = other.nrOfChildren;
		byteSize = other.byteSize;

		triangleIndices = other.triangleIndices;

		for (UINT i = 0; i < nrOfChildren; i++)
			childrenIndices[i] = other.childrenIndices[i];
	}
	
	AABB & operator=(const AABB & other)
	{
		if (this != &other)
		{
			position = other.position;
			axis = other.axis;
			level = other.level;
			nrOfChildren = other.nrOfChildren;
			byteSize = other.byteSize;

			triangleIndices = other.triangleIndices;

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
		str += "nrOfChildren: " + std::to_string(nrOfChildren) + "\n";
		str += "ChildrenIndices: ";
		for (UINT i = 0; i < nrOfChildren; i++)
			str += std::to_string(childrenIndices[i]) + ", ";
		str += "\n";
		str += "nrOfTriangles: " + std::to_string(triangleIndices.size()) + "\n";
		str += "triangleIndices: ";
		for (UINT i = 0; i < triangleIndices.size(); i++)
			str += std::to_string(triangleIndices[i]) + ", ";
		str += "\n";
		str += "ByteSize: " + std::to_string(byteSize) + "\n";
		return str;
	}

#pragma endregion

	void CalcSize()
	{
		byteSize = sizeof(*this);
		byteSize -= sizeof(std::vector<UINT>);

		if (!triangleIndices.empty())
		{
			byteSize += triangleIndices.size() * sizeof(UINT);
		}
		
	}

	UINT				byteSize = 0;
	
	DirectX::XMFLOAT3	position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3	axis = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
	
	UINT				level = 0;
	UINT				nrOfChildren = 0;
	UINT				childrenIndices[8] = {0};

	std::vector<UINT>	triangleIndices;
};

class OcTree
{
public:
	OcTree();
	~OcTree();

	void BuildTree(std::vector<STRUCTS::Triangle> & triangles, UINT treeLevel = 3, UINT worldSize = 1024);
	const UINT & GetTotalTreeByteSize() const;
	const std::vector<AABB> & GetTree() const;

private:
	std::vector<AABB>	m_ocTree;
	std::vector<size_t>	m_leafIndex;

	UINT				m_totalTreeByteSize = 0;
	void _BuildTree(const DirectX::XMFLOAT3 & startPos, const UINT & level, const UINT & maxLevel, const UINT & worldSize, const size_t & parentIndex);

	bool _inside(const AABB & aabb, const STRUCTS::Triangle & tri);
	bool _pointInside(const DirectX::XMFLOAT3 & min, const DirectX::XMFLOAT3 & max, const DirectX::XMFLOAT3 & point);

};

