#pragma once
#include "Window/Window.h"
#include "Leaf-Drop/CoreRender.h"

#include "Leaf-Drop/Objects/StaticMesh.h"
#include "Leaf-Drop/Objects/Drawable.h"
#include "Leaf-Drop/Objects/Texture.h"
#include "Leaf-Drop/Utillity/Timer.h"
#include "Leaf-Drop/Objects/Camera.h"

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

	void ClearGPU();
	void Release();

	BOOL Running() const;
};

