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
	

	return true;
}

std::vector<STRUCTS::StaticVertex>* StaticMesh::GetMesh()
{
	return &m_mesh;
}