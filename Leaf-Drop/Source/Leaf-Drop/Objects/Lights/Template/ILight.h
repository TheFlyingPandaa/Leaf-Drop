#pragma once
#include "../../Transform.h"
#include <DirectXMath.h>
#include "../../../CoreRender.h"

class Window;

class ILight : public Transform
{
protected:
	enum LightType
	{
		Point = 0,
		Directional = 1
	};
public:
	virtual ~ILight();
	void Queue();

	void SetIntensity(const float & intensity);
	const float & GetIntensity() const;
			
	void SetColor(const DirectX::XMFLOAT4 & color);
	void SetColor(const float & x, const float & y, const float & z, const float & w = 1.0f);
	const DirectX::XMFLOAT4 & GetColor() const;


	virtual const UINT & GetType() const;

protected:
	ILight(const LightType & lightType);
	CoreRender * p_coreRender;
	const Window * p_window;

	LightType p_lightType = LightType::Point;
			
private:
	DirectX::XMFLOAT4 m_color = DirectX::XMFLOAT4(1, 1, 1, 1);
	
	UINT m_lightTypeInteger;
	float m_intensity = 1;
};

