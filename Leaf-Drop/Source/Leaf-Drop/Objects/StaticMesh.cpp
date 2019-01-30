#include "CorePCH.h"
#include "StaticMesh.h"
#include <assimp/Importer.hpp>     
#include <assimp/scene.h>          
#include <assimp/postprocess.h>

static DirectX::XMFLOAT4 Convert_Assimp_To_DirectX(const aiVector3D & vec, const float & w = 1.0f)
{
	return DirectX::XMFLOAT4(vec.x, vec.y, vec.z, w);
}

static DirectX::XMFLOAT2 Convert_Assimp_To_DirectX2(const aiVector3D & vec)
{
	return DirectX::XMFLOAT2(vec.x, vec.y);
}

StaticMesh::StaticMesh()
{
}


StaticMesh::~StaticMesh()
{
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_vertexUploadBuffer);
}

bool StaticMesh::LoadMesh(const std::string & path)
{
	Assimp::Importer importer;

	const aiScene * scene = importer.ReadFile(path.c_str(),
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_SortByPType |
		aiProcess_FlipUVs);

	if (!scene || !scene->HasMeshes())
	{
		return false;
	}
	m_mesh.clear();

	for (UINT i = 0; i < scene->mNumMeshes; i++)
	{
		for (UINT j = 0; j < scene->mMeshes[i]->mNumVertices; j++)
		{
			STRUCTS::StaticVertex vertex = {};
			
			vertex.Position = Convert_Assimp_To_DirectX(scene->mMeshes[i]->mVertices[j]);
			vertex.Normal = Convert_Assimp_To_DirectX(scene->mMeshes[i]->mNormals[j], 0);
			vertex.Tangent = Convert_Assimp_To_DirectX(scene->mMeshes[i]->mTangents[j], 0);
			vertex.biTangent = Convert_Assimp_To_DirectX(scene->mMeshes[i]->mBitangents[j], 0);
			vertex.UV = Convert_Assimp_To_DirectX2(scene->mMeshes[i]->mTextureCoords[0][j]);
			m_mesh.push_back(vertex);
		}
	}
	
	CoreRender * coreRender = CoreRender::GetInstance();
	ID3D12GraphicsCommandList * commandList = coreRender->GetCommandList()[coreRender->GetFrameIndex()];
	
	if (SUCCEEDED(coreRender->OpenCommandList()))
	{
		m_vertexBufferSize = static_cast<UINT>(sizeof(STRUCTS::StaticVertex) * this->m_mesh.size());

		if (SUCCEEDED(coreRender->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer))))
		{
			SET_NAME(m_vertexBuffer, std::wstring(std::wstring(L"StaticMesh :") + 
				std::wstring(path.begin(), path.end()) +
				std::wstring(L": vertexBuffer")).c_str());

			if (SUCCEEDED(coreRender->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_vertexUploadBuffer))))
			{
				SET_NAME(m_vertexBuffer, std::wstring(std::wstring(L"StaticMesh :") +
					std::wstring(path.begin(), path.end()) +
					std::wstring(L": vertexUploadBuffer")).c_str());

				D3D12_SUBRESOURCE_DATA vertexData = {};
				vertexData.pData = reinterpret_cast<void*>(this->m_mesh.data());
				vertexData.RowPitch = m_vertexBufferSize;
				vertexData.SlicePitch = m_vertexBufferSize;

				UpdateSubresources(commandList, m_vertexBuffer, m_vertexUploadBuffer, 0, 0, 1, &vertexData);

				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
				commandList->Close();
				if (SUCCEEDED(coreRender->ExecuteCommandList()))
				{
					m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
					m_vertexBufferView.StrideInBytes = sizeof(STRUCTS::StaticVertex);
					m_vertexBufferView.SizeInBytes = m_vertexBufferSize;
					return true;
				}
			}
		}
	}

	return true;
}

std::vector<STRUCTS::StaticVertex>* StaticMesh::GetMesh()
{
	return &m_mesh;
}

const D3D12_VERTEX_BUFFER_VIEW & StaticMesh::GetVBV() const
{
	return this->m_vertexBufferView;
}