#pragma once
#include "Template/IRender.h"
class GeometryPass : public IRender
{
public:
	GeometryPass();
	~GeometryPass();

	// Inherited via IRender
	virtual HRESULT Init() override;
	virtual void Update() override;
	virtual void Draw() override;
	virtual void Release() override;


};

