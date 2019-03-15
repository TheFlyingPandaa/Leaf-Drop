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
		DirectX::XMFLOAT4 UV;
	};
	
	struct LIGHT_VALUES
	{
		DirectX::XMUINT4 Type;
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT4 Color;
		union
		{
			DirectX::XMFLOAT4 Direction;
			DirectX::XMFLOAT4 Point;
		};
	};

	struct ObjectValues
	{
		DirectX::XMFLOAT4X4A World; //might need fence bobby please fix
		DirectX::XMFLOAT4X4A WorldInverse;
		DirectX::XMFLOAT3 Min;
		DirectX::XMFLOAT3 Max;
		UINT MeshIndex = 0;
		UINT TextureIndex = 0;
	};
}