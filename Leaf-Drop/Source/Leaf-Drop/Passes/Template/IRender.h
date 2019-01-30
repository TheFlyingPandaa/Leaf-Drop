#pragma once

class CoreRender;
class Window;
class Drawable;
class Light;

class IRender
{
public:
	virtual ~IRender();
	
	void Submit(Drawable * drawable);
	void SubmitLight(Light * light);
protected:
	IRender();

	CoreRender * p_coreRender;
	Window * p_window;

	virtual HRESULT Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Release() = 0;

};

