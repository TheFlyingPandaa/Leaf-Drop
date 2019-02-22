#include "CorePCH.h"
#include "Drawable.h"
#include "StaticMesh.h"

Drawable::Drawable()
{
	m_coreRenderer = CoreRender::GetInstance();
}


Drawable::~Drawable()
{
}

void Drawable::SetMesh(StaticMesh * staticMesh)
{
	this->m_mesh = staticMesh;
}

StaticMesh * Drawable::GetMesh() const
{
	return m_mesh;
}

void Drawable::SetTexture(Texture * texture)
{
	m_texture = texture;
}

Texture * Drawable::GetTexture() const
{
	return m_texture;
}

void Drawable::SetMetallic(Texture* texture)
{
	this->m_metallic = texture;
}

Texture* Drawable::GetMetallic() const
{
	return this->m_metallic;
}

void Drawable::SetNormal(Texture* texture)
{
	this->m_normal = texture;
}

Texture* Drawable::GetNormal() const
{
	return this->m_normal;
}

void Drawable::SetColor(const DirectX::XMFLOAT4A & color)
{
	m_color = color;
}

void Drawable::SetColor(const float & r, const float & g, const float & b, const float & a)
{
	SetColor({ r, g, b, a });
}

const DirectX::XMFLOAT4A & Drawable::GetColor() const
{
	return m_color;
}

void Drawable::Draw()
{
	m_coreRenderer->GetGeometryPass()->Submit(this);
	m_coreRenderer->GetComputePass()->Submit(this);
	m_coreRenderer->GetPrePass()->Submit(this);
}
