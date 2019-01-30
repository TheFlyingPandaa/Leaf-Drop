#include "CorePCH.h"
#include "IRender.h"



IRender::IRender()
{
	p_coreRender = CoreRender::GetInstance();
	p_window = Window::GetInstance();
}


IRender::~IRender()
{
}

void IRender::Submit(Drawable * drawable)
{
}

void IRender::SubmitLight(Light * light)
{
}
