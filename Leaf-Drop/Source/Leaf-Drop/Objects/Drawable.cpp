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

void Drawable::Draw()
{
	m_coreRenderer->GetGeometryPass()->Submit(this);
}