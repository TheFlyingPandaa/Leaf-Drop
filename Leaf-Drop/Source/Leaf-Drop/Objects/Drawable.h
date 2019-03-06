#pragma once
#include "Transform.h"

class StaticMesh;
class Texture;

class Drawable : public Transform
{
public:
	Drawable();
	~Drawable();
	
	void SetMesh(StaticMesh * staticMesh);
	StaticMesh * GetMesh() const;
	
	void SetTexture(Texture * texture);
	Texture * GetTexture() const;

	void SetMetallic(Texture * texture);
	Texture * GetMetallic() const;

	void SetNormal(Texture * texture);
	Texture * GetNormal() const;
	
	void SetColor(const DirectX::XMFLOAT4A &color);
	void SetColor(const float & r, const float & g, const float & b, const float & a = 1.0f);
	const DirectX::XMFLOAT4A & GetColor() const;

	void SetAsStatic();

	bool isStatic() const;

	void Draw();

	void SetTextureOffset(const UINT & offset = 0);
	const UINT & GetTextureOffset() const;

private:
	StaticMesh * m_mesh = nullptr;
	CoreRender * m_coreRenderer = nullptr;
	Texture	* m_texture = nullptr;
	Texture	* m_metallic = nullptr;
	Texture	* m_normal = nullptr;
	DirectX::XMFLOAT4A m_color = {1.0f, 1.0f, 1.0f, 1.0f};
	bool m_isStatic = false;

	UINT m_textureOffset = 0;
};

