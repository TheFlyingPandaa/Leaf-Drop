#pragma once
#include "Template/IRender.h"
class RayDefinePass : public IRender
{
public:
	RayDefinePass();
	~RayDefinePass();

	// Inherited via IRender
	virtual HRESULT Init() override;
	virtual void Update() override;
	virtual void Draw() override;
	virtual void Release() override;
};

