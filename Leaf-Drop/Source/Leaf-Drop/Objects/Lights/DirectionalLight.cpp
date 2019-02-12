#include "CorePCH.h"
#include "DirectionalLight.h"


void DirectionalLight::SetDirection(const DirectX::XMFLOAT4& direction)
{
	this->m_direction = direction;
}

void DirectionalLight::SetDirection(const float& x, const float& y, const float& z, const float& w)
{
	this->SetDirection(DirectX::XMFLOAT4(x, y, z, w));
}

const DirectX::XMFLOAT4& DirectionalLight::GetDirection() const
{
	return this->m_direction;
}

