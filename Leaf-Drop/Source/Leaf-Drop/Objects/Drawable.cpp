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