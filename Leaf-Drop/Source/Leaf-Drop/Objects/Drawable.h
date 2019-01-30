#pragma once
#include "Transform.h"

class StaticMesh;

class Drawable : public Transform
{
public:
	Drawable();
	~Drawable();
	
	void SetMesh(StaticMesh * staticMesh);
	StaticMesh * GetMesh() const;
	
	void Draw();

private:
	StaticMesh * m_mesh = nullptr;
	CoreRender * m_coreRenderer = nullptr;

};

