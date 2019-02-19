#include "CorePCH.h"
#include "DirectionalLight.h"


DirectionalLight::DirectionalLight() : ILight(Directional)
{

}

void DirectionalLight::SetDirection(const DirectX::XMFLOAT4& direction)
{
	using namespace DirectX;

	XMVECTOR vDir = XMLoadFloat4(&direction);
	vDir = XMVector3Normalize(vDir);
	XMStoreFloat4(&this->m_direction, vDir);
}

void DirectionalLight::SetDirection(const float& x, const float& y, const float& z, const float& w)
{
	this->SetDirection(DirectX::XMFLOAT4(x, y, z, w));
}

const DirectX::XMFLOAT4& DirectionalLight::GetDirection() const
{
	return this->m_direction;
}

