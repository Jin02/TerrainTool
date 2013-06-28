#pragma once

#include <Windows.h>

#define START_ENABLE(iMessage)\
	if(iMessage==WM_INITDIALOG ) return true;\
	if( AVDirector::GetDiector()->IsStart() == false ) return false

class AVDialogBox
{
protected:
	HWND			m_hWnd;
	HWND			m_parentHandle;
	HINSTANCE		m_hInst;
	UINT			m_ID;
	DLGPROC			m_dlgProc;

public:
	AVDialogBox(HINSTANCE hInst, HWND parentHandle, UINT IDD, DLGPROC proc);
	~AVDialogBox(void);

	void Create();
	void Show(int x, int y, int w, int h);
	void Delete();
	void SetHidden(bool b);

protected:
	void _SetPosTrackBar(UINT id, int pos);
	void _SetRangeTrackBar(UINT id, int min, int max);
	void _SetEdtValue(UINT id, int num);
	void _SetCheckBox(UINT id, bool b);
	wchar_t* _PathPassing(wchar_t *path);
	int _GetSelTxt(UINT id, wchar_t *str);


public:
	static bool CALLBACK WndProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
};

