#pragma once
#include <Windows.h>
#include <string>
#include <thread>

struct Input
{
public:
	enum KEY_CODE
	{
		BACKSPACE = 8,
		TAB = 9,
		ENTER = 13,
		SHIFT = 16,
		CTRL = 17,
		ALT = 18,
		PAUSEBREAK = 19,
		CAPS_LOCK = 20,
		ESCAPE = 27,
		SPACE = 32,
		PAGE_UP = 33,
		PAGE_DOWN = 34,
		END = 35,
		HOME = 36,
		LEFT_ARROW = 37,
		UP_ARROW = 38,
		RIGHT_ARROW = 39,
		DOWN_ARROW = 40,
		INSERT = 45,
		DELETE_KEY = 46,
		KEY_0 = 48,
		KEY_1 = 49,
		KEY_2 = 50,
		KEY_3 = 51,
		KEY_4 = 52,
		KEY_5 = 53,
		KEY_6 = 54,
		KEY_7 = 55,
		KEY_8 = 56,
		KEY_9 = 57,
		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,
		LEFT_WINDOW_KEY = 91,
		RIGHT_WINDOW_KEY = 92,
		SELECT_KEY = 93,
		NUMPAD_0 = 96,
		NUMPAD_1 = 97,
		NUMPAD_2 = 98,
		NUMPAD_3 = 99,
		NUMPAD_4 = 100,
		NUMPAD_5 = 101,
		NUMPAD_6 = 102,
		NUMPAD_7 = 103,
		NUMPAD_8 = 104,
		NUMPAD_9 = 105,
		MULTIPLY = 106,
		ADD = 107,
		SUBTRACT = 109,
		DECIMAL_POINT = 110,
		DIVIDE = 111,
		F1 = 112,
		F2 = 113,
		F3 = 114,
		F4 = 115,
		F5 = 116,
		F6 = 117,
		F7 = 118,
		F8 = 119,
		F9 = 120,
		F10 = 121,
		F11 = 122,
		F12 = 123,
		NUM_LOCK = 144,
		SCROLL_LOCK = 145,
		SEMI_COLON = 186,
		EQUAL_SIGN = 187,
		COMMA = 188,
		DASH = 189,
		PERIOD = 190,
		FORWARD_SLASH = 191,
		GRAVE_ACCENT = 192,
		OPEN_BRACKET = 219,
		BACK_SLASH = 220,
		CLOSE_BRAKET = 221,
		SINGLE_QUOTE = 222
	};
	enum MOUSE_CODE
	{
		LBUTTON = 1,
		MBUTTON = 16,
		RBUTTON = 2
	};
};


class Window
{
public:
	static Window * GetInstance();

	BOOL Create(HINSTANCE hInstance, INT ShowWnd, UINT width, UINT height, BOOL fullscreen = FALSE, const std::string & windowName = "myWindow", const std::string & windowTitle = "myTitle");
	
	BOOL IsOpen() const;
	
	void SetWindowTitle(const std::string & windowTitle);

	POINT GetWindowSize() const;

	BOOL IsKeyPressed(int key);
	BOOL IsMousePressed(int button);

	HWND GetHwnd() const;

	BOOL IsFullscreen() const;

	void Close();

	POINT GetMousePosition();
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
	bool m_mousePress[256]	= { false };

private:
	Window();
	virtual ~Window();
	static LRESULT CALLBACK _WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void _procMessages();
	void _run();
	void _create(HINSTANCE hInstance, INT ShowWnd);

};