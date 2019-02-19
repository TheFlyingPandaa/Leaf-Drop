#include "CorePCH.h"
#include "Window.h"

Window::Window()
{
	m_hwnd = NULL;
	m_height = 0;
	m_width = 0;
	m_fullscreen = 0;
	m_isOpen = FALSE;
	m_creationError = FALSE;
}

Window::~Window()
{
	
}

Window * Window::GetInstance()
{
	static Window window;
	return &window;
}

BOOL Window::Create(HINSTANCE hInstance, INT ShowWnd, UINT width, UINT height, BOOL fullscreen, const std::string & windowName, const std::string & windowTitle)
{
	if (m_isOpen)
		return TRUE;

	m_windowName = std::wstring(windowName.begin(), windowName.end());
	m_windowTitle = std::wstring(windowTitle.begin(), windowTitle.end());
	m_fullscreen = fullscreen;
	m_width = width;
	m_height = height;


	m_windowThread = std::thread(&Window::_create, this, hInstance, ShowWnd);

	while (m_creationError == FALSE && m_isOpen == FALSE);


	

	

	

	return m_isOpen;
}

BOOL Window::IsOpen() const
{
	return m_isOpen;
}

void Window::SetWindowTitle(const std::string & windowTitle)
{
	m_windowTitle = std::wstring(windowTitle.begin(), windowTitle.end());
	SetWindowText(m_hwnd, m_windowTitle.c_str());
}

POINT Window::GetWindowSize() const
{
	POINT p;
	p.x = m_width;
	p.y = m_height;
	return p;
}

BOOL Window::IsKeyPressed(int key)
{
	return m_keyPress[key];
}

BOOL Window::IsMousePressed(int button)
{
	return m_mousePress[button];
}

HWND Window::GetHwnd() const
{
	return m_hwnd;
}

BOOL Window::IsFullscreen() const
{
	return m_fullscreen;
}

void Window::Close()
{	
	static bool isclosed = false;
	if (!isclosed)
	{
		isclosed = true;
		m_isOpen = FALSE;
		while (!m_windowThread.joinable());
		m_windowThread.join();
		DestroyWindow(m_hwnd);
	}
}

POINT Window::GetMousePosition()
{
	POINT p;

	GetCursorPos(&p);
	ScreenToClient(m_hwnd, &p);
	return p;

}

void Window::ResetMouse()
{
	POINT p;

	p.x = m_width / 2;
	p.y = m_height / 2;

	ClientToScreen(m_hwnd, &p);
	SetCursorPos(p.x, p.y);
}

LRESULT Window::_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window * wnd = Window::GetInstance();

	switch (msg)
	{
	case WM_KEYDOWN:
		wnd->m_keyPress[wParam] = true;
		return 0;

	case WM_KEYUP:
		wnd->m_keyPress[wParam] = false;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		wnd->m_mousePress[wParam] = true;
		return 0;

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		wnd->m_mousePress[wParam] = false;
		return 0;

	case WM_DESTROY:
		wnd->m_isOpen = FALSE;
		return 0;
	}

	return DefWindowProc(hWnd,
		msg,
		wParam,
		lParam);
}

void Window::_procMessages()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			m_isOpen = FALSE;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Window::_run()
{
	while (m_isOpen)
	{
		_procMessages();
	}
}

void Window::_create(HINSTANCE hInstance, INT ShowWnd)
{
	RECT r;
	r.top = 0;
	r.left = 0;
	r.right = m_width;
	r.bottom = m_height;
	if (m_fullscreen)
	{
		HMONITOR hmon = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);

		m_width = mi.rcMonitor.right - mi.rcMonitor.left;
		m_height = mi.rcMonitor.bottom - mi.rcMonitor.top;
		r.right = m_width;
		r.bottom = m_height;
	}
	else
	{
		AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);
	}
	WNDCLASSEX wc;


	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = _WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_windowName.c_str();
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Error registering class",
			L"Error", MB_OK | MB_ICONERROR);
		m_creationError = TRUE;
		return;
	}

	m_hwnd = CreateWindowEx(NULL,
		m_windowName.c_str(),
		m_windowTitle.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		r.right - r.left, r.bottom - r.top,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!m_hwnd)
	{
		MessageBox(NULL, L"Error creating window",
			L"Error", MB_OK | MB_ICONERROR);
		m_creationError = TRUE;
		return;
	}

	if (m_fullscreen)
	{
		SetWindowLong(m_hwnd, GWL_STYLE, 0);
	}
	/*else
	{
		RECT rect;
		GetWindowRect(m_hwnd, &rect);
		AdjustWindowRect(&rect, WS_CHILD, FALSE);
	}*/
	


	ShowWindow(m_hwnd, ShowWnd);
	UpdateWindow(m_hwnd);


	m_isOpen = TRUE;
	
	_run();

}

