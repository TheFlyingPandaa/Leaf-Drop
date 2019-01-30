#pragma once
#include <EASTL/vector.h>
#include <vector>

#include "../Extern/Extern.h"
class StaticMesh
{

public:
	StaticMesh();
	~StaticMesh();

	bool LoadMesh(const std::string & path);
	std::vector<STRUCTS::StaticVertex> * GetMesh();
private:
	
	std::vector<STRUCTS::StaticVertex> m_mesh;
};

