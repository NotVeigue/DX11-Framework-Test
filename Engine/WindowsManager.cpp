#include "WindowsManager.h"
#include "InputManager.h"


// ----------------------------------------------------------------------------------
// Singleton Stuff
// ----------------------------------------------------------------------------------
template<> WindowsManager* Singleton<WindowsManager>::msSingleton = 0;
WindowsManager& WindowsManager::GetSingleton(void)
{
	assert(msSingleton);
	return *msSingleton;
}

WindowsManager* WindowsManager::GetSingletonPtr(void)
{
	return msSingleton;
}


// ----------------------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------------------
WindowsManager::WindowsManager(int clientWidth, int clientHeight, bool vSync, bool windowed)
{
	m_clientWidth = clientWidth;
	m_clientHeight = clientHeight;
	m_vSync = vSync;
	m_windowed = windowed;

	WindowsManager::GetSingleton().InitializeWindows(clientWidth, clientHeight);
}

WindowsManager::~WindowsManager()
{
	WindowsManager::GetSingleton().ShutdownWindows();
}


bool WindowsManager::InitializeWindows(int clientWidth, int clientHeight)
{
	WNDCLASSEX wc;
	//DEVMODE dmScreenSettings;
	int posX, posY;

	m_hinstance = GetModuleHandle(NULL);
	m_applicationName = "Windows Manager";

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);
	RegisterClassEx(&wc);

	posX = (GetSystemMetrics(SM_CXSCREEN) - clientWidth) / 2;
	posY = (GetSystemMetrics(SM_CYSCREEN) - clientHeight) / 2;

	RECT windowRect = { 0, 0, clientWidth, clientHeight };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, "EngineWindow",
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP | WS_CAPTION,
		posX, posY, windowRect.right - windowRect.left, 
		windowRect.bottom - windowRect.top, 
		NULL, NULL, m_hinstance, NULL);

	if (!m_hwnd)
	{
		MessageBoxA(NULL, "Could not create the render window.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	return true;
}

void WindowsManager::ShutdownWindows()
{
	// Fix the display settings if leaving full screen mode.
	if (false)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	return;
}

int WindowsManager::GetClientWidth(void) const
{
	return m_clientWidth;
}

int WindowsManager::GetClientHeight(void) const
{
	return m_clientHeight;
}

HWND WindowsManager::GetWindowHandle(void) const
{
	return m_hwnd;
}

bool WindowsManager::GetVSync(void) const
{
	return m_vSync;
}

bool WindowsManager::GetWindowed(void) const
{
	return m_windowed;
}

LRESULT CALLBACK WindowsManager::HandleMessage(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	if (umsg == WM_KEYDOWN)
	{
		// The 30th bit of the lparam tells us whether the key was previously pressed or not.
		// Since we are keeping track of held keys on our own, we only care about the first
		// time a key is pressed.
		if((static_cast<unsigned int>(lparam) & (1 << 30)) == 0)
			InputManager::GetSingleton().SetKeyDown(static_cast<unsigned int>(wparam));
		return 0;
	}
	else if (umsg == WM_KEYUP)
	{
		InputManager::GetSingleton().SetKeyUp(static_cast<unsigned int>(wparam));
		return 0;
	}

	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	// Check if the window is being destroyed or closed.
	if(umsg == WM_DESTROY || umsg == WM_CLOSE)
	{
		PostQuitMessage(0);
		return 0;
	}
	// All other messages pass to the message handler in the system class.
	else
	{
		return WindowsManager::GetSingleton().HandleMessage(hwnd, umsg, wparam, lparam);
	}
}
