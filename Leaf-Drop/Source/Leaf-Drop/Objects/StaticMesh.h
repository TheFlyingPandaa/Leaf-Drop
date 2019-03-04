#pragma once

#include <vector>
#include "../Extern/Extern.h"

class StaticMesh
{
public:
	static const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> & GetCpuHandles();
	static void BindCompute(const UINT & rootSignatureIndex, ID3D12GraphicsCommandList * commandList);

	struct MIN_MAX_AABB
	{
		DirectX::XMFLOAT3 min = {};
		DirectX::XMFLOAT3 max = {};
		UINT meshIndex = 0;
	};

public:
	StaticMesh();
	~StaticMesh();

	bool LoadMesh(const std::string & path);
	std::vector<STRUCTS::StaticVertex> * GetRawVertices();
	UINT GetNumberOfVertices() const;

	const MIN_MAX_AABB & GetAABB() const;

	const D3D12_VERTEX_BUFFER_VIEW & GetVBV() const;

	void Release();

private:
	UINT m_vertexBufferSize = 0;
	ID3D12Resource * m_vertexBuffer = nullptr;
	ID3D12Resource * m_vertexUploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	MIN_MAX_AABB m_aabb;

	std::vector<STRUCTS::StaticVertex> m_mesh;


	ConstantBuffer m_meshBindlessBuffer;

	static ConstantBuffer	s_bindlessMeshes;
	static UINT				s_offset;
	static std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> s_cpuHandles;

private:
	void _calcMinMax();


};

