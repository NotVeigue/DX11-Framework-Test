#pragma once
#include "Singleton.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class WindowsManager : public Singleton<WindowsManager>
{
private:
	bool InitializeWindows(int, int);
	void ShutdownWindows();

	int m_clientWidth;
	int m_clientHeight;

	LPCSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;
	bool m_vSync;
	bool m_windowed;

public:
	WindowsManager(int clientWidth, int clientHeight, bool vSync, bool windowed);
	~WindowsManager();

	int  GetClientWidth(void) const;
	int	 GetClientHeight(void) const;
	HWND GetWindowHandle(void) const;
	bool GetVSync(void) const;
	bool GetWindowed(void) const;

	static WindowsManager& GetSingleton(void);
	static WindowsManager* GetSingletonPtr(void);

	LRESULT CALLBACK HandleMessage(HWND, UINT, WPARAM, LPARAM);
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

