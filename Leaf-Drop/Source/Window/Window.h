#pragma once
#include <Windows.h>
#include <string>
#include <thread>

struct UINT2
{
	UINT x, y;
};

class Window
{
public:

public:
	static Window * GetInstance();

	BOOL Create(HINSTANCE hInstance, INT ShowWnd, UINT width, UINT height, BOOL fullscreen = FALSE, const std::string & windowName = "myWindow", const std::string & windowTitle = "myTitle");
	
	BOOL IsOpen() const;
	
	void SetWindowTitle(const std::string & windowTitle);

	UINT2 GetWindowSize() const;

	BOOL IsKeyPressed(int key);

	HWND GetHwnd() const;

	BOOL IsFullscreen() const;

	void Close();

	UINT2 GetMousePosition();
	void ResetMouse();

private:
	HWND	m_hwnd;
	UINT	m_height;
	UINT	m_width;
	BOOL	m_fullscreen;
	BOOL	m_isOpen;
	BOOL	m_creationError;

	std::wstring m_windowName;
	std::wstring m_windowTitle;

	std::thread m_windowThread;

	bool m_keyPress[256]	= { false };

private:
	Window();
	virtual ~Window();
	static LRESULT CALLBACK _WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void _procMessages();
	void _run();
	void _create(HINSTANCE hInstance, INT ShowWnd);

};