#pragma once
#include "Template/ILight.h"


class PointLight :
	public ILight
{

public:
	PointLight();

	void SetDropOff(const float & dropOff);
	void SetPow(const float & pow);
	void SetRadius(const float & radius);

	const float & GetDropOff() const;
	const float & GetPow() const;
	const float & GetRadius() const;
	
private:
	float m_dropOff = 2;
	float m_pow = 2;

	float m_radius = FLT_MAX;

};

