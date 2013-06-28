#include "AVDialogBox.h"
#include <commCtrl.h>

AVDialogBox::AVDialogBox(HINSTANCE hInst, HWND parentHandle, UINT IDD , DLGPROC proc)
{
	m_hWnd = NULL;
	m_parentHandle = parentHandle;
	m_hInst = hInst;
	m_ID = IDD;
	m_dlgProc = proc;
}

AVDialogBox::~AVDialogBox(void)
{
	Delete();
}

int AVDialogBox::_GetSelTxt(UINT id, wchar_t *str)
{
	HWND hWnd = GetDlgItem(m_hWnd, id);

	int count = SendMessage(hWnd, LB_GETCURSEL,0,0);
	SendMessage(hWnd, LB_GETTEXT, count, (LPARAM)str);

	return count;
}

wchar_t* AVDialogBox::_PathPassing(wchar_t *path)
{
	wchar_t txt[256];
	wcscpy(txt,	wcsrchr(path, L'/')+1);
	return txt;
}

void AVDialogBox::Create()
{
	m_hWnd = CreateDialog(m_hInst, MAKEINTRESOURCE(m_ID), m_parentHandle, m_dlgProc);
}

void AVDialogBox::Show(int x, int y, int w, int h)
{
	MoveWindow(m_hWnd, x, y, w, h, true);
	ShowWindow(m_hWnd, SW_SHOW);
}

void AVDialogBox::SetHidden(bool b)
{
	ShowWindow(m_hWnd, b == true ? SW_HIDE : SW_SHOWDEFAULT);
}

void AVDialogBox::_SetCheckBox(UINT id, bool b)
{
	unsigned int res = b == true ? BST_CHECKED : BST_UNCHECKED;
	HWND hWnd = GetDlgItem(m_hWnd, id);
	SendMessage(hWnd, BM_SETCHECK, res, 0);
}

void AVDialogBox::_SetPosTrackBar(UINT id, int pos)
{
	int min, max;
	HWND hWnd = GetDlgItem(m_hWnd, id);
	min = SendMessage(hWnd, TBM_GETRANGEMIN, 0, 0);
	max = SendMessage(hWnd, TBM_GETRANGEMAX, 0, 0);

	if(pos < min)		pos = min;
	if(pos > max)		pos = max;

	SendMessage(hWnd, TBM_SETPOS, true, pos);
}

void AVDialogBox::_SetRangeTrackBar(UINT id, int min, int max)
{
	HWND hWnd = GetDlgItem(m_hWnd, id);
	SendMessage(hWnd, TBM_SETRANGE, FALSE, MAKELPARAM(min, max));
}

void AVDialogBox::_SetEdtValue(UINT id, int num)
{
	wchar_t txt[30] = L"0";
	HWND hWnd = GetDlgItem(m_hWnd, id);
	wsprintf(txt, L"%d", num);
	SetWindowText(hWnd, txt);
}

void AVDialogBox::Delete()
{
	if(m_hWnd != NULL) {
	EndDialog(m_hWnd, NULL);
	m_hWnd = NULL;
	}
}

bool CALLBACK AVDialogBox::WndProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	if( iMessage == WM_CREATE )
		return true;

	return false;
}