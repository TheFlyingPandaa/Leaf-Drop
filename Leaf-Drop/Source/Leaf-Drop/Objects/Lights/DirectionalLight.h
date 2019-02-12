#pragma once
#include "Template/ILight.h"

class DirectionalLight : 
	public ILight
{
public:
	DirectionalLight();

	void SetDirection(const DirectX::XMFLOAT4 & direction);
	void SetDirection(const float & x, const float & y, const float & z, const float & w = 0.0f);

	const DirectX::XMFLOAT4 & GetDirection() const;

private:
	DirectX::XMFLOAT4 m_direction;
};
