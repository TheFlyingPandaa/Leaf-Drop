#include "CorePCH.h"
#include "PointLight.h"


PointLight::PointLight() : ILight(Point)
{
}

void PointLight::SetDropOff(const float& dropOff)
{
	this->m_dropOff = dropOff;
}

void PointLight::SetPow(const float& pow)
{
	this->m_pow = pow;
}

void PointLight::SetRadius(const float& radius)
{
	this->m_radius = radius;
}

const float& PointLight::GetRadius() const
{
	return this->m_radius;
}

const float& PointLight::GetDropOff() const
{
	return this->m_dropOff;
}

const float& PointLight::GetPow() const
{
	return this->m_pow;
}
