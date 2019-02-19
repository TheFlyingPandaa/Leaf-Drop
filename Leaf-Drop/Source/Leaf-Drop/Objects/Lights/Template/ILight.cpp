#include "CorePCH.h"
#include "ILight.h"
#include "Source/Window/Window.h"

#pragma warning (disable : 4172)

ILight::ILight(const LightType& lightType)
{

	p_coreRender = CoreRender::GetInstance();
	p_window = Window::GetInstance();

	this->p_lightType = lightType;

	this->m_lightTypeInteger = lightType;

	this->m_intensity = 1;
	this->m_color = DirectX::XMFLOAT4(1, 1, 1, 1);
}

ILight::~ILight()
{

}

void ILight::Queue()
{
	if (m_intensity > 0)
	{
		p_coreRender->GetDeferredPass()->SubmitLight(this);
		p_coreRender->GetComputePass()->SubmitLight(this);
	}
}

void ILight::SetIntensity(const float& intensity)
{
	this->m_intensity = intensity;
}

const float& ILight::GetIntensity() const
{
	return this->m_intensity;
}

void ILight::SetColor(const DirectX::XMFLOAT4& color)
{
	this->m_color = color;
}

void ILight::SetColor(const float& x, const float& y, const float& z, const float& w)
{
	this->SetColor(DirectX::XMFLOAT4(x, y, z, w));
}

const DirectX::XMFLOAT4 & ILight::GetColor() const
{
	return this->m_color;
}

const UINT& ILight::GetType() const
{
	return m_lightTypeInteger;
}








