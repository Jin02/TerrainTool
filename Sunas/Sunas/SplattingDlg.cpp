#include "SplattingDlg.h"
#include "resource.h"
#include "SharedTexture.h"
#include "toolTypeDef.h"
#include "AVDirector.h"

static CSplattingDlg *pSplattingDlg;

CSplattingDlg::CSplattingDlg(HINSTANCE hInst, HWND parentHandle)
	:AVDialogBox(hInst, parentHandle, IDD_DLG_TEXTURE, (DLGPROC)CSplattingDlg::_SplattingProc)
{
	pSplattingDlg = this;
	m_nID = 0;
}

CSplattingDlg::~CSplattingDlg(void)
{
	m_preview.Release();
}

void CSplattingDlg::ContentInit(int baseImgID, int loadSplatNum)
{
	_ClearListBox(IDC_LIST_NEW_TEXTURE);
	_ClearListBox(IDC_LIST_SEL);

	m_vID.clear();
	m_nID = loadSplatNum;

	m_preview.InitDevice(GetDlgItem(m_hWnd, IDC_STATIC_2), 256, 256);
	m_preview.SetPosition(0,0);

	HWND hWnd = GetDlgItem(m_hWnd, IDC_LIST_TEXTURE_ALLLIST);

	for(int i=0; i < TILE_TEXTURE_NUM; ++i)
	{
		wchar_t txt[256];
		wchar_t *path = SharedTexture::GetSharedTexture()->GetTexture(i)->path;

		wcscpy(txt, _PathPassing(path));

		SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)txt);
	}

	wchar_t *path = SharedTexture::GetSharedTexture()->GetTexture(baseImgID)->path;

	m_preview.SetTexture(path);

	wchar_t txt[256];
	wcscpy(txt, _PathPassing(path));
	hWnd = GetDlgItem(m_hWnd, IDC_LIST_SEL);
	SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)txt);
}

void CSplattingDlg::_SelListBox(UINT id, UINT msg)
{
	if(msg != LBN_SELCHANGE ) return;

	Scn_MapTool *scn = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene());
	wchar_t str[30];
	int sel = _GetSelTxt(id,str);
	
	if( sel == 0 && id == IDC_LIST_SEL )
	{
		MessageBox(m_hWnd, L"기본 베이스 텍스쳐는 사용할수없습니다", L"알림", MB_OK);
		SendMessage(GetDlgItem(m_hWnd, IDC_LIST_SEL), LB_SETCURSEL, -1, 0);
		return;
	}

	if(id == IDC_LIST_SEL)
		scn->SelSplatTex(m_vID[sel-1]);

	wchar_t *path = 
		SharedTexture::GetSharedTexture()->GetTextureN(str)->path;

	m_preview.SetTexture(path);
	InvalidateRect(m_hWnd, NULL, true);
	InvalidateRect(GetDlgItem(m_hWnd, IDC_STATIC_2), NULL, true);
}

void CSplattingDlg::_TextureDelete()
{
	Scn_MapTool *scn = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene());
	wchar_t selTxt[30];
	int count = _GetSelTxt(IDC_LIST_SEL, selTxt);

	if( count == 0 )
	{
		MessageBox(m_hWnd, L"기본 타일은 지울수 없습니다.", L"오류", MB_OK);
		return;
	}

	scn->DeleteSplattingTex(m_vID[count-1]);
	m_vID.erase( m_vID.begin() + count-1 );

	HWND hWnd = GetDlgItem(m_hWnd, IDC_LIST_SEL);
	SendMessage(hWnd, LB_DELETESTRING, count, 0);

	scn->SetSplatOption(SPLATTING_OPTION_NONE);

	hWnd = GetDlgItem(m_hWnd, IDC_BT_TEXTURE_ALPHA_ADD);
	SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);

	hWnd = GetDlgItem(m_hWnd, IDC_BT_TEXTURE_ALPHA_MINUS);
	SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
}

void CSplattingDlg::_TextureAdd()
{
	Scn_MapTool *scn = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene());
	wchar_t selTxt[30];
	int sel = _GetSelTxt(IDC_LIST_TEXTURE_ALLLIST, selTxt);
//	scn->vSplattingTexID().push_back(count);
	HWND hWnd = GetDlgItem(m_hWnd, IDC_LIST_SEL);
//	int num = SendMessage(hWnd, LB_GETCOUNT, 0, 0);
	
	if(scn->AddSplattingTex(sel, m_nID) == false)
	{
		MessageBox(m_hWnd, L"최대 갯수 초과 입니다", L"알림", MB_OK);
		return;
	}

	m_vID.push_back(m_nID);
	++m_nID;
	SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)selTxt);
}

void CSplattingDlg::TextureAdd(int index)
{
	Scn_MapTool *scn = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene());
	wchar_t txt[30];
	HWND hWnd = GetDlgItem(m_hWnd, IDC_LIST_TEXTURE_ALLLIST);
	SendMessage(hWnd, LB_GETTEXT, index, (LPARAM)txt);

	hWnd = GetDlgItem(m_hWnd, IDC_LIST_SEL);
	SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)txt);
}

bool CALLBACK CSplattingDlg::_SplattingProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	Scn_MapTool *scn = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene());

	START_ENABLE(iMessage);

	switch(iMessage)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BT_TEXTURE_ALPHA_ADD:
//			CToolInformation::GetToolInfo()->splatOption = SPLATTING_OPTION_ADD;
			scn->SetSplatOption(SPLATTING_OPTION_ADD);
			SetFocus(*AVDirector::GetDiector()->GetApplication()->GetHwnd());
			break;

		case IDC_BT_TEXTURE_ALPHA_MINUS:
			//CToolInformation::GetToolInfo()->splatOption = SPLATTING_OPTION_MINUS;
			scn->SetSplatOption(SPLATTING_OPTION_MINUS);
			SetFocus(*AVDirector::GetDiector()->GetApplication()->GetHwnd());
			break;

		case IDC_BT_TEXTURE_DELETE:
			pSplattingDlg->_TextureDelete();
			SetFocus(*AVDirector::GetDiector()->GetApplication()->GetHwnd());
			break;
		case IDC_BT_TEXTURE_ADD:
			pSplattingDlg->_TextureAdd();
			SetFocus(*AVDirector::GetDiector()->GetApplication()->GetHwnd());
			break;
		case IDC_LIST_SEL:
		case IDC_LIST_TEXTURE_ALLLIST:
			pSplattingDlg->_SelListBox(LOWORD(wParam),HIWORD(wParam));
			break;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		pSplattingDlg->m_preview.Render();
		EndPaint(hDlg, &ps);
		break;

	case WM_DESTROY:
		
		break;
	}

//	if( iMessage == WM_COMMAND )
		

	//SetFocus(*AVDirector::GetDiector()->GetApplication()->GetHwnd());

	return false;
}


void CSplattingDlg::Reset()
{
	HWND hWnd = GetDlgItem(m_hWnd, IDC_BT_TEXTURE_ALPHA_ADD);
	SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
	hWnd = GetDlgItem(m_hWnd, IDC_BT_TEXTURE_ALPHA_MINUS);
	SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
}

void CSplattingDlg::_ClearListBox(unsigned int id)
{
	HWND hWnd = GetDlgItem(m_hWnd, id);

	while( SendMessage(hWnd, LB_GETCOUNT, 0, 0) )
		SendMessage(hWnd, LB_DELETESTRING, 0, 0);
}