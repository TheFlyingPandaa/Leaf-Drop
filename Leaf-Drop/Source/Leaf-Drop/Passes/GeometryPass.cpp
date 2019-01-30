#include "CorePCH.h"
#include "GeometryPass.h"


GeometryPass::GeometryPass() : IRender()
{
}


GeometryPass::~GeometryPass()
{
}

HRESULT GeometryPass::Init()
{
	HRESULT hr = 0;
	if (FAILED(hr = p_CreateCommandList()))
	{
		return hr;
	}

	if (FAILED(hr = OpenCommandList()))
	{
		return hr;
	}

	if (FAILED(hr = _init()))
	{
		return hr;
	}

	return hr;
}

void GeometryPass::Update()
{
	if (FAILED(OpenCommandList()))
	{
		return;
	}

	

}

void GeometryPass::Draw()
{
}

void GeometryPass::Release()
{
}

HRESULT GeometryPass::_init()
{
	HRESULT hr = 0;



	return hr;
}
