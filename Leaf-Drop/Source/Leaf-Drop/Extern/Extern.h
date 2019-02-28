#pragma once

#include <Windows.h>
#include <comdef.h>
#include <DirectXMath.h>

const UINT FRAME_BUFFER_COUNT = 3U;
const FLOAT CLEAR_COLOR[] = { 0,0,0,0 };

namespace DEBUG
{

	extern HRESULT CreateError(const std::string& errorMsg);
	extern HRESULT CreateError(const std::wstring& errorMsg);
	extern HRESULT CreateError(const LPCWSTR& errorMsg);
	extern HRESULT CreateError(const HRESULT& hr);
}

namespace STRUCTS
{
	struct StaticVertex
	{
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT4 Normal;
		DirectX::XMFLOAT4 Tangent;
		DirectX::XMFLOAT4 biTangent;
		DirectX::XMFLOAT2 UV;
	};
	struct Vertex
	{
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT4 normal;
		DirectX::XMFLOAT4 tangent;
		DirectX::XMFLOAT4 bitangent;
		DirectX::XMFLOAT2 uv;
	};

	struct Triangle
	{
		Vertex v1, v2, v3;
		UINT textureIndexStart;
	};

	struct OctreeValues
	{
		DirectX::XMFLOAT4X4A WorldInverse;
		DirectX::XMFLOAT3 Min;
		DirectX::XMFLOAT3 Max;
		UINT MeshIndex = 0;
	};

}