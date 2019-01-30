#pragma once
#include "Window/Window.h"
#include "Leaf-Drop/CoreRender.h"

#include "Leaf-Drop/Objects/StaticMesh.h"
#include "Leaf-Drop/Objects/Drawable.h"

class Core
{
private:
	Window * m_window = nullptr;
	CoreRender * m_coreRenderer = nullptr;
public:
	Core();
	~Core();

	HRESULT Init(HINSTANCE hInstance);

	HRESULT Flush();

	BOOL Running() const;
};

