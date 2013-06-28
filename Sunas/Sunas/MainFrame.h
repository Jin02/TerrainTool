#pragma once

#include "AVWindow.h"
#include "AVDirector.h"
#include "TerrainTool.h"
#include "NewMap.h"


class CMainFrame : public AVWindow
{
	HINSTANCE				 m_hIsnt;
	AVDirector				*m_director;
	CTerrainTool			*m_Tool;
	bool					 m_bFirst;
	bool					m_bisLoad;
	wchar_t					m_strPath[256];

public:
	std::vector<int>		m_splatTexID;


public:
	CMainFrame(AVRECT &rect, HINSTANCE hInst);
	~CMainFrame(void);

private:
	void	_LoadTextures();
	void	_MenuFileNew();
	void	_MenuFileSave(HWND hWnd);
	void	_MenuFileLoad(HWND hWnd);
	void	_MenuExit();

private:
	static bool CALLBACK newMapProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

public:
	void   Run();
	void   loadTerrainFile(wchar_t *path);
	static LRESULT WINAPI MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};