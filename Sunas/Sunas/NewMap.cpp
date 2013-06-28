#include "NewMap.h"
#include "resource.h"
#include <commCtrl.h>
#include <math.h>
#include "SharedTexture.h"
#include "toolTypeDef.h"

static CNewMap *newMap;

CNewMap::CNewMap(HINSTANCE hInst, HWND hParentHandle)
	:AVDialogBox(hInst, hParentHandle, IDD_DLG_NEW, (DLGPROC)CNewMap::_newMapProc)
{
	newMap = this;
	m_nScale = 10;
	m_nTileSize = 0;
	m_nCellSize = 0;
	m_isCreate = false;
	m_nSelTexID = -1;
}

CNewMap::~CNewMap(void)
{
	m_preview.Release();
}

void CNewMap::Show()
{
	//NULL
}

void CNewMap::Create()
{
	//NULL
}

void CNewMap::DoModal()
{
	DialogBox(m_hInst, MAKEINTRESOURCE(m_ID), m_parentHandle, m_dlgProc);
}

void CNewMap::_Init()
{
	_SetRangeTrackBar(IDC_SLIDE_SCALE, 10, 20);
	_SetEdtValue(IDC_EDIT_SCALE, 10);

	HWND hWnd = GetDlgItem(m_hWnd, IDC_LIST_NEW_TEXTURE);

	for(int i=0; i < TILE_TEXTURE_NUM; ++i)
	{
		wchar_t txt[256];
		wchar_t *path = SharedTexture::GetSharedTexture()->GetTexture(i)->path;

		wcscpy(txt, _PathPassing(path));

		SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)txt);
	}

	m_preview.InitDevice(GetDlgItem(m_hWnd, IDC_STATIC_1),256,256);
	m_preview.SetPosition(0,0);
}

void CNewMap::_SlideBar(HWND hWnd)
{
	int pos = SendMessage(hWnd, TBM_GETPOS, 0, 0);

	m_nScale = pos;
	_SetEdtValue(IDC_EDIT_SCALE, pos);
}

void CNewMap::_btCancel(HWND hDlg)
{
	EndDialog(hDlg, NULL);
}

void CNewMap::_btCreate(HWND hDlg)
{	
	m_isCreate = true;

	if(m_nSelTexID == -1)
	{
		MessageBox(m_hWnd, L"기본 텍스쳐를 선택 하시지 않으셨습니다", L"오류", MB_OK);
		return;
	}

	if(m_nCellSize == 0)
	{
		MessageBox(m_hWnd, L"Cell 크기를 입력하시지 않으셨습니다", L"오류", MB_OK);
		return;		
	}

	if(m_nTileSize == 0)
	{
		MessageBox(m_hWnd, L"Tile 크기를 입력하시지 않으셨습니다", L"오류", MB_OK);
		return;
	}

	EndDialog(hDlg, NULL);
}

void CNewMap::_btAdd()
{
}

void CNewMap::_infoTxtSet()
{
	int MapSize, Triangles, Textures, Vertex;

	MapSize		= m_nTileSize * m_nCellSize;
	Vertex		= (MapSize+1) * (MapSize+1);
	Triangles	= MapSize * MapSize * 2;

	wchar_t str[30];

	wsprintf(str, L"%d x %d", MapSize, MapSize);
	SetDlgItemText(m_hWnd, IDC_TXT_MAP_SIZE, str);

	wsprintf(str, L"%d", Vertex);
	SetDlgItemText(m_hWnd, IDC_TXT_VERTEX, str);

	wsprintf(str, L"%d", Triangles);
	SetDlgItemText(m_hWnd, IDC_TXT_TRIANGLES, str);
}

void CNewMap::_calcCell(int n)
{
	m_nCellSize = pow(2.f, n);
	_infoTxtSet();
}

void CNewMap::_calcTile(int n)
{			
	m_nTileSize = pow(2.f, n);
	_infoTxtSet();
}

void CNewMap::_SelListBox(UINT id, UINT msg)
{
	if(msg != LBN_SELCHANGE ) return;

	wchar_t str[30];
	_GetSelTxt(id,str);

	TEXTURE *tex = SharedTexture::GetSharedTexture()->GetTextureN(str);
	wchar_t *path = tex->path;

	//조사해야해 여기서 nSEltexid를,
	//어떻게 알지? 어떻게? 어떻게?는 시발
	m_nSelTexID = tex->id;
	m_preview.SetTexture(path);
	InvalidateRect(m_hWnd, NULL, true);
	InvalidateRect(GetDlgItem(m_hWnd, IDC_STATIC_1), NULL, true);
}

bool CALLBACK CNewMap::_newMapProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	int pow;

	switch(iMessage)
	{
	case WM_INITDIALOG:
		int x, y, w, h;
		RECT parentRect, dlgRect;
		newMap->m_hWnd = hDlg;

		GetWindowRect(newMap->m_parentHandle, &parentRect);
		GetClientRect(hDlg, &dlgRect);

		x = parentRect.left + (parentRect.right - parentRect.left) / 2 - (dlgRect.right - dlgRect.left) / 2;
		y = parentRect.top+100;
		w = dlgRect.right - dlgRect.left;
		h = dlgRect.bottom - dlgRect.top;

		MoveWindow(hDlg, x, y, w, h, true);
		newMap->_Init();

		return 0;

	case WM_CLOSE:
		newMap->_btCancel(hDlg);
		return false;

	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
		case IDC_BT_CANCEL:
			newMap->_btCancel(hDlg);
			break;
		case IDC_BT_CREATE:
			newMap->_btCreate(hDlg);
			break;

		case IDC_RADIO_CELL_4:
		case IDC_RADIO_CELL_8:
		case IDC_RADIO_CELL_16:
			pow = LOWORD(wParam) - IDC_RADIO_CELL_4 + 2;
			newMap->_calcCell(pow);
			break;

		case IDC_RADIO_TILE_8:
		case IDC_RADIO_TILE_16:
		case IDC_RADIO_TILE_32:
			pow = LOWORD(wParam) - IDC_RADIO_TILE_8 + 3;
			newMap->_calcTile(pow);
			break;

		case IDC_LIST_NEW_TEXTURE:
			newMap->_SelListBox(LOWORD(wParam), HIWORD(wParam));
			break;
		}

		break;

	case WM_HSCROLL:
		newMap->_SlideBar(reinterpret_cast<HWND>(lParam));
		break;

	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		newMap->m_preview.Render();
		EndPaint(hDlg, &ps);
		break;
	}

	return false;
}