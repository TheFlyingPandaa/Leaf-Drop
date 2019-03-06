#pragma once
#include <vector>
#include <sal.h>

#define INCREMENT_LEVEL 8

#define VECTOR_TYPE UINT

struct AABB
{
#pragma region stuff
	AABB() {};
	AABB(const AABB & other)
	{
		Min = other.Min;
		Max = other.Max;
		level = other.level;
		nrOfChildren = other.nrOfChildren;
		byteSize = other.byteSize;
		byteStart = other.byteStart;

		meshDataIndices = other.meshDataIndices;
		nrOfObjects = other.nrOfObjects;

		for (UINT i = 0; i < nrOfChildren; i++)
		{
			childrenIndices[i] = other.childrenIndices[i];
			childrenByteAddress[i] = other.childrenByteAddress[i];
		}
	}
	
	AABB & operator=(const AABB & other)
	{
		if (this != &other)
		{
			
			Min = other.Min;
			Max = other.Max;
			level = other.level;
			nrOfChildren = other.nrOfChildren;
			byteSize = other.byteSize;
			byteStart = other.byteStart;

			meshDataIndices = other.meshDataIndices;
			nrOfObjects = other.nrOfObjects;

			for (UINT i = 0; i < nrOfChildren; i++)
			{
				childrenIndices[i] = other.childrenIndices[i];
				childrenByteAddress[i] = other.childrenByteAddress[i];
			}
		}
		return *this;
	}

	std::string ToString() const
	{
		std::string str = "";
		str += "Min: " + std::to_string(Min.x) + ", " + std::to_string(Min.y) + ", " + std::to_string(Min.z) + "\n";
		str += "Max: " + std::to_string(Max.x) + ", " + std::to_string(Max.y) + ", " + std::to_string(Max.z) + "\n";
		str += "Level: " + std::to_string(level) + "\n";
		str += "nrOfChildren: " + std::to_string(nrOfChildren) + "\n";
		str += "ChildrenIndices: ";
		for (UINT i = 0; i < nrOfChildren; i++)
			str += std::to_string(childrenIndices[i]) + ", ";
		str += "\n";
		str += "ChildrenByteAddress: ";
		for (UINT i = 0; i < nrOfChildren; i++)
			str += std::to_string(childrenByteAddress[i]) + ", ";
		str += "\n";
		str += "nrOfObjects: " + std::to_string(nrOfObjects) + "\n";
		str += "ObjectIndices: ";
		for (UINT i = 0; i < meshDataIndices.size(); i++)
			str += std::to_string(meshDataIndices[i]) + ", ";
		str += "\n";
		str += "ByteSize: " + std::to_string(byteSize) + "\n";
		str += "ByteStart: " + std::to_string(byteStart) + "\n";
		return str;
	}

#pragma endregion

	void CalcSize()
	{
		byteSize = sizeof(*this);
		byteSize -= sizeof(meshDataIndices) + sizeof(VECTOR_TYPE);

		if (!meshDataIndices.empty())
		{
			byteSize += (UINT)meshDataIndices.size() * sizeof(VECTOR_TYPE);
		}
		
	}

	UINT				byteSize = 0;
	UINT				byteStart = 0;

	DirectX::XMFLOAT3	Min = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3	Max = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
	
	UINT				level = 0;
	UINT				nrOfChildren = 0;
	UINT				childrenIndices[8] = { 0 };
	UINT				childrenByteAddress[8] = {0};

	UINT				nrOfObjects = 0;

	std::vector<VECTOR_TYPE>	meshDataIndices;
};

class OcTree
{
public:
	OcTree();
	~OcTree();

	void BuildTree(INT startX = 0, INT startY = 0, INT startZ = 0, UINT treeLevel = 3, UINT worldSize = 1024);
	void BuildTree(const DirectX::XMINT3 & startPos = DirectX::XMINT3(0,0,0), UINT treeLevel = 3, UINT worldSize = 1024);


	/* If the function "Merge" will be called to another tree, set the bool value to true  */
	void PlaceObjects(const std::vector<STRUCTS::MeshValues> & MeshValues, bool willRecieveAMerge = false);

	/* Very important that the trees has the same leve, worldSize and startPosition */
	void Merge(const OcTree & other);

	const UINT & GetTotalTreeByteSize() const;
	const std::vector<AABB> & GetTree() const;


	std::string ToString() const;

private:
	std::vector<AABB>	m_ocTree;
	std::vector<UINT>	m_leafIndices;
	UINT				m_leafCounter = 0;

	UINT				m_numberOfObjectsInLeafs = 0;

	UINT				m_maxLevel = 0;

	UINT				m_totalTreeByteSize = 0;

	bool _inside(const AABB & aabb, const STRUCTS::MeshValues & colVal);
	bool _pointInside(const DirectX::XMFLOAT3 & min, const DirectX::XMFLOAT3 & max, const DirectX::XMFLOAT3 & point);

	void _CreateAABB(const DirectX::XMFLOAT3 & position, const DirectX::XMFLOAT3 & size, UINT level, bool isLeaf, UINT index);
	UINT _GetAABBIndexByWorldPos(const DirectX::XMFLOAT3 & worldPos, UINT Level);

	void _clearLeafs();
	void _traverseAndPlace(const STRUCTS::MeshValues & meshVal, UINT meshIndex, UINT ocIndex);


};

