#pragma once
#include "Transform.h"

struct StaticVertex;

class Drawable : public Transform
{
public:
	Drawable();
	~Drawable();
	
	void SetMesh(const StaticVertex *const* data, const UINT & size);
		
};

