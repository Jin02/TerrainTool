#include "AVWindow.h"

AVWindow::AVWindow(AVRECT &rect, HINSTANCE hInst, bool isChild, wchar_t *windowName, HWND parentHandle, WNDPROC proc, wchar_t *menuName)
{
	m_window.cbSize			= sizeof(WNDCLASSEX);
	m_window.style			= CS_CLASSDC;
	m_window.hInstance		= hInst;//GetModuleHandle(NULL);
	m_window.cbClsExtra		= 0L;
	m_window.cbWndExtra		= 0L;
	m_window.hIcon			= NULL;
	m_window.hCursor		= NULL;
	m_window.hbrBackground	= NULL;
	m_window.hIconSm		= NULL;
	m_window.lpszMenuName	= menuName;//MAKEINTRESOURCE(IDR_MENU1);
	m_window.lpszClassName	= windowName;
	
	if(proc == NULL)
		m_window.lpfnWndProc	= AVWindow::WndProc; //need to fix
	else
		m_window.lpfnWndProc	= proc;

	if(isChild)
		m_windowOption		= WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN;
	else
		m_windowOption		= WS_OVERLAPPEDWINDOW | WS_SYSMENU;

	m_rect					= rect;
	m_pWindowsName			= windowName;
	m_parentHandle			= parentHandle;
}

AVWindow::~AVWindow(void)
{
	Destroy();
}

void AVWindow::Create()
{
	RegisterClassEx(&m_window);

	//wchar_t *className = m_pWindowsName;

	//if(AVWindow::isChild())
	//	className = NULL;

	m_hWnd = CreateWindow(m_pWindowsName, m_pWindowsName, m_windowOption, 
		static_cast<int>(m_rect.x), static_cast<int>(m_rect.y), 
		static_cast<int>(m_rect.w), static_cast<int>(m_rect.h), 
		m_parentHandle, NULL, m_window.hInstance, NULL);
}

void AVWindow::Destroy()
{
	UnRegist();

	if( m_windowOption != (WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN) )
		DestroyWindow(m_hWnd);
	else
		PostQuitMessage(0);
}

void AVWindow::Show()
{
	if( m_windowOption != (WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN) )
		ShowWindow(m_hWnd, SW_SHOWDEFAULT);
}

void AVWindow::UnRegist()
{
	UnregisterClass( m_pWindowsName, m_window.hInstance );
}

bool AVWindow::isChild()
{
	return (m_windowOption == (WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN));
}

LRESULT WINAPI AVWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc( hWnd, msg, wParam, lParam );
}