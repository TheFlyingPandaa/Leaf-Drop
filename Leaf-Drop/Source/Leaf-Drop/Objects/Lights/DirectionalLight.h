#pragma once
#include "Template/ILight.h"

class DirectionalLight : 
	public ILight
{
public:
	void SetDirection(const DirectX::XMFLOAT4 & direction) const;
	void SetDirection(const float & x, const float & y, const float & z, const float & w = 0.0f) const;

private:

};
