#pragma once

#include "AVDialogBox.h"

//
//#define SLIDER	0
//#define EDITBOX	1

//�� �޸� ������.. �ϴ� �̷��� ó������ ��

class CEditDlg : public AVDialogBox
{
private:

public:
	CEditDlg(HINSTANCE hInst, HWND parentHandle);
	~CEditDlg(void);

private:
	void _EditBox(UINT msg, UINT id);
	void _SlideBar(HWND hWnd);

public:
	void ContentInit();

private:
	static bool CALLBACK _EditProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
};

