#pragma once

#include <Windows.h>
#include <comdef.h>
#include <DirectXMath.h>

const UINT FRAME_BUFFER_COUNT = 3U;

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
		DirectX::XMFLOAT4 position;
	};
}