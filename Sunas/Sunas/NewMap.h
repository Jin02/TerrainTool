#pragma once

#include "AVDialogBox.h"
#include "AVSTPreview.h"

class CNewMap : public AVDialogBox
{
private:
	int m_nScale;
	int m_nTileSize;
	int m_nCellSize;
	int m_nSelTexID;

	bool m_isCreate;
	AVSTPreview m_preview;

public:
	CNewMap(HINSTANCE hInst, HWND hParentHandle);
	~CNewMap(void);

public:
	void Create();
	void Show();
	void DoModal();
	bool GetisCreate() { return m_isCreate; }

private:
	void _SlideBar(HWND hWnd);
	void _Init();
	void _btCancel(HWND hDlg);
	void _btCreate(HWND hDlg);
	void _btAdd();
	void _infoTxtSet();
	void _calcCell(int n);
	void _calcTile(int n);
	void _SelListBox(UINT id, UINT msg);

public:
	int GetScale()		{ return m_nScale; }
	int GetTileSize()	{ return m_nTileSize; }
	int GetCellSize()	{ return m_nCellSize; }
	int GetSelTexID()	{ return m_nSelTexID; }

private:
	static bool CALLBACK _newMapProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
};

