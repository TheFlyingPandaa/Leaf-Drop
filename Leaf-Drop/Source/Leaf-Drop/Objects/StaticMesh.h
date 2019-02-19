#pragma once
//#include <EASTL/vector.h>
#include <vector>

#include "../Extern/Extern.h"
class StaticMesh
{

public:
	StaticMesh();
	~StaticMesh();

	bool LoadMesh(const std::string & path);
	std::vector<STRUCTS::StaticVertex> * GetRawVertices();
	UINT GetNumberOfVertices() const;

	const D3D12_VERTEX_BUFFER_VIEW & GetVBV() const;

	void Release();

private:
	UINT m_vertexBufferSize = 0;
	ID3D12Resource * m_vertexBuffer = nullptr;
	ID3D12Resource * m_vertexUploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	std::vector<STRUCTS::StaticVertex> m_mesh;
};

