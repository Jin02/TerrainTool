#pragma once

#include <windows.h>
#include "Information.h"

class AVWindow
{
protected:
	WNDCLASSEX				 m_window;
	UINT					 m_windowOption;
	AVRECT					 m_rect;
	wchar_t					*m_pWindowsName;
	HWND					 m_parentHandle;
	HWND					 m_hWnd;

public:
	AVWindow(
		AVRECT &rect, 
		HINSTANCE hInst, 
		bool isChild, 
		wchar_t *windowName,
		HWND parentHandle,
		WNDPROC proc = NULL, 
		wchar_t *menuName = NULL);
	virtual ~AVWindow(void);

public:
	void	Create();
	void	Show();
	void	Destroy();
	void	UnRegist();

	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool isChild();
};
