#pragma once

#include <vector>
#include "AVDialogBox.h"
#include "AVSTPreview.h"

class CSplattingDlg : public AVDialogBox
{
private:
	AVSTPreview			m_preview;
	int					 m_nID;

public:	
	std::vector<int>	 m_vID;

public:
	CSplattingDlg(HINSTANCE hInst, HWND parentHandle);
	~CSplattingDlg(void);

private:
	void _TextureDelete();
	void _TextureAdd();
	void _SelListBox(UINT id, UINT msg);

	void _ClearListBox(unsigned int id);

public:
	void ContentInit(int baseImgID, int loadSplatNum = 0);
	void Reset();
	void TextureAdd(int index);

private:
	static bool CALLBACK _SplattingProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
};
