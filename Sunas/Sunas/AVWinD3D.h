#pragma once

#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#include <d3d9.h>
#include "AVWindow.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
//#pragma comment(lib, "winmm.lib")

class AVWinD3D : public AVWindow
{
	LPDIRECT3D9             m_pD3D;
	LPDIRECT3DDEVICE9       m_pd3dDevice;

	D3DPRESENT_PARAMETERS	m_d3dpp;

public:
	AVWinD3D(
		WNDPROC wndProc,		//¿÷¡ˆ
		HINSTANCE hInst,		//¿’±∏
		AVRECT &windowRect,		//¿’æÓ
		HWND parentHandle = NULL,
		wchar_t *pWindowName = L"Hello World", //π’∞Ì 
		bool isChild = false,	//¿÷æÓ
		wchar_t *menuName = NULL); //√ﬂ∞°

	~AVWinD3D(void);

	void AVWinD3D::InitWinD3D(wchar_t *application, bool zBuffer, bool lighting, DWORD cullModeValue);

public:
	inline LPDIRECT3D9			GetD3D()		{ return m_pD3D;		}
	inline LPDIRECT3DDEVICE9	GetD3DDevice()	{ return m_pd3dDevice;	}
	inline HWND*				GetHwnd()		{ return &m_hWnd;		}
	inline AVRECT&				GetWindowSize() { return AVWindow::m_rect;	}
};