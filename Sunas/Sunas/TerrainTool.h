#pragma once

#include "resource.h"
#include "AVDialogBox.h"
#include "EditDlg.h"
#include "SplattingDlg.h"

class CTerrainTool : public AVDialogBox
{
private:
	//����� �� ��Ʈ���� Dlg������, �̰����� �����ϵ��� ����
	HWND			m_Tab;
	CEditDlg		*m_pEditDlg;
	CSplattingDlg	*m_pSplattingDlg;

	bool			m_bPick;

public:
	CTerrainTool(HINSTANCE hInst, HWND parentHandle);
	~CTerrainTool(void);
	
private:
	void _TapControl();
	void _EditBox(UINT msg, UINT id);
	void _SlideBar(HWND hWnd);

public:
	void Create();
	void Show(int x, int y, int w, int h);
	void ContentInit();
	void SplatDlgTextureAdd(int i);

public:
	static bool CALLBACK ToolWndProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
};

