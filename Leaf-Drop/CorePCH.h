#pragma once
#include <new>
#include <stdint.h>

//This is for EASTL
//ole dole doff.
//Kinke lane koff
//Koffe lane binke bane ole dole doff

//inline void* operator new[](size_t size, const char* pName, int flags, unsigned     debugFlags, const char* file, int line)
//{
//	return malloc(size);
//}
//
//inline void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
//{
//	return malloc(size);
//}
inline void* __cdecl operator new[](size_t size, const char* pName, int flags, unsigned     debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}
inline void* __cdecl operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}


#include <Windows.h>

#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "DXGI.lib")

#include "Source/Window/Window.h"
#include "Source/Leaf-Drop/CoreRender.h"

#include "Source/Leaf-Drop/Wrappers/d3dx12.h"

#ifdef _DEBUG
#include <iostream>

#define PRINT(p) {std::cout << p << std::endl;}
#else
#define PRINT(p);
#endif

#define SAFE_RELEASE(p) {if ((p)) {(p)->Release(); (p) = nullptr;}}
#define SAFE_DELETE(p) {if ((p)) {delete (p); (p) = nullptr;}}

#define SET_NAME(p, str) {if ((p)){ (p)->SetName((str));}}



	
