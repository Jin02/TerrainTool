#include "TerrainTool.h"
#include <commCtrl.h>
#include "Information.h"
#include "AVDirector.h"
#include "ToolInformation.h"

static CTerrainTool *pTerrainTool;

CTerrainTool::CTerrainTool(HINSTANCE hInst, HWND parentHandle)
	:AVDialogBox(hInst, parentHandle, IDD_DLG_CONTROL, (DLGPROC)CTerrainTool::ToolWndProc)
{
	m_Tab			= NULL;
	pTerrainTool	= this;
	m_pEditDlg		= NULL;

	m_bPick			= true;

	m_pEditDlg		= NULL;
	m_pSplattingDlg	= NULL;
}

CTerrainTool::~CTerrainTool(void)
{
	if(m_pEditDlg)
		delete m_pEditDlg;
	m_pEditDlg = NULL;

	SAFE_DELETE(m_pEditDlg);
	SAFE_DELETE(m_pSplattingDlg);
}

void CTerrainTool::Create()
{
	AVDialogBox::Create();

	TCITEM		tie;

	m_Tab  = GetDlgItem(m_hWnd, IDC_TAB);

	tie.mask	= TCIF_TEXT;
	tie.pszText = L"Height";
	TabCtrl_InsertItem(m_Tab, 0, &tie);

	tie.pszText = L"Splatting";
	TabCtrl_InsertItem(m_Tab, 1, &tie);

	m_pEditDlg = new CEditDlg(m_hInst, m_hWnd);
	m_pEditDlg->Create();

	m_pSplattingDlg = new CSplattingDlg(m_hInst, m_hWnd);
	m_pSplattingDlg->Create();//temp¿”Ω√¿”Ω√ΩÃπ‘§±Ω√
}

void CTerrainTool::ContentInit()
{
	_SetEdtValue(IDC_EDIT_BRUSH_SIZE, 10);
	_SetEdtValue(IDC_EDIT_CAMERA_MOVE, 1);
	_SetRangeTrackBar(IDC_SLIDE_BRUSH_SIZE, 1, 100);
	_SetRangeTrackBar(IDC_SLIDE_CAMERA_MOVE, 1, 20);
	_SetPosTrackBar(IDC_SLIDE_BRUSH_SIZE, 10);
	_SetPosTrackBar(IDC_SLIDE_CAMERA_MOVE, 1);
	_SetCheckBox(IDC_CHECK_FOG, false);
	_SetCheckBox(IDC_CHECK_LIGHT, false);
	_SetCheckBox(IDC_CHECK_WIRE, false);
	_SetCheckBox(IDC_CHECK_PICK, true);

	m_pEditDlg->ContentInit();
	int id = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene())->baseTexID();
	int num = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene())->SizeSplatTextures();
	m_pSplattingDlg->ContentInit(id, num);//¿”Ω√
	
	for(int i=0; i<num; i++)
		m_pSplattingDlg->m_vID.push_back(i);
	
	HWND hWnd = GetDlgItem(m_hWnd, IDC_TAB);
	TabCtrl_SetCurSel(hWnd, 0);
}

void CTerrainTool::SplatDlgTextureAdd(int i)
{	
	m_pSplattingDlg->TextureAdd(i);
}

void CTerrainTool::Show(int x, int y, int w, int h)
{
	AVDialogBox::Show(x, y, w, h);

	m_pEditDlg->Show(3,20, 297, 375);
	m_pSplattingDlg->Show(3,20,297,360);
	m_pSplattingDlg->SetHidden(true);
}

void CTerrainTool::_EditBox(UINT msg, UINT id)
{
	if(EN_CHANGE != msg) return;
	Scn_MapTool *scn = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene());

	int value = GetDlgItemInt(m_hWnd, id, NULL, true);

	if(value<1)
	{
		value = 1;
		_SetEdtValue(id, value);
		MessageBox(m_parentHandle, L"text", L"caption", MB_OK);
	}
	
	if( value > 20 && id == IDC_EDIT_CAMERA_MOVE )
	{
		value = 20;
		_SetEdtValue(id, value);
		MessageBox(m_parentHandle, L"text", L"caption", MB_OK);
	}

	if( id == IDC_EDIT_BRUSH_SIZE )
	{
		scn->SetBrushSize(value);
//		CToolInformation::GetToolInfo()->nBrushSize	= value; 
		_SetPosTrackBar(IDC_SLIDE_BRUSH_SIZE, value);
	}
	else
	{
		_SetPosTrackBar(IDC_SLIDE_CAMERA_MOVE, value);
//		AVDirector::GetDiector()->nCameraSpeed = value;
		scn->SetCameraSpeed(value);
	}
}

void CTerrainTool::_SlideBar(HWND hWnd)
{
	int pos = SendMessage(hWnd, TBM_GETPOS, 0, 0);
	Scn_MapTool *scn = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene());

	if( hWnd == GetDlgItem(m_hWnd, IDC_SLIDE_BRUSH_SIZE) )
	{
		scn->SetBrushSize(pos);
//		CToolInformation::GetToolInfo()->nBrushSize = pos;
		_SetEdtValue(IDC_EDIT_BRUSH_SIZE, pos);
	}
	else
	{
		_SetEdtValue(IDC_EDIT_CAMERA_MOVE, pos);
//		AVDirector::GetDiector()->nCameraSpeed = pos;
		scn->SetCameraSpeed(pos);
	}
}

void CTerrainTool::_TapControl()
{
	int sel = TabCtrl_GetCurSel(m_Tab);
	Scn_MapTool *scn = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene());

	if(sel == 0)
	{
		m_pSplattingDlg->SetHidden(true);
		m_pEditDlg->SetHidden(false);
		scn->SetSplatOption(SPLATTING_OPTION_NONE);
	}
	else
	{
		m_pSplattingDlg->SetHidden(false);
		m_pEditDlg->SetHidden(true);
		m_pSplattingDlg->Reset();
	}
}

bool CALLBACK CTerrainTool::ToolWndProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	Scn_MapTool *scn = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene());
	HWND hWnd = GetDlgItem(hDlg, LOWORD(wParam));
	bool b = (SendMessage(hWnd, BM_GETCHECK,0 ,0) == BST_CHECKED);

	START_ENABLE(iMessage);

	switch(iMessage) {
	case WM_NOTIFY:
		if(((LPNMHDR)lParam)->code == TCN_SELCHANGE)
			pTerrainTool->_TapControl();
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BT_CIRCLE:
		case IDC_BT_SQUARE:
			scn->SetBrushType( LOWORD(wParam) == IDC_BT_CIRCLE ? BRUSH_TYPE_CIRCLE : BRUSH_TYPE_SQUARE);
			break;

		case IDC_EDIT_BRUSH_SIZE:
		case IDC_EDIT_CAMERA_MOVE:
			pTerrainTool->_EditBox(HIWORD(wParam), LOWORD(wParam));
			break;

		case IDC_CHECK_FOG: 
//			b = (SendMessage(hWnd, BM_GETCHECK,0 ,0) == BST_CHECKED);
			scn->SetIsFog(b);
			break;

		case IDC_CHECK_LIGHT:
//			AVDirector::GetDiector()->bLight = !AVDirector::GetDiector()->bLight;
			scn->SetIsLight(b);
			break;

		case IDC_CHECK_WIRE:
//			AVDirector::GetDiector()->bWireframe = !AVDirector::GetDiector()->bWireframe;
			scn->SetIsWire(b);
			break;
		}

		break;

	case WM_HSCROLL:
		pTerrainTool->_SlideBar(reinterpret_cast<HWND>(lParam));
		break;

//	case WM_PAINT:
//		HWND hWnd = GetDlgItem(hDlg, IDC_SLIDE_CAMERA_MOVE);
//		hdc = BeginPaint(hWnd, &ps);
////		SetBkColor(hdc,RGB(255,0,0));
//		SetBkMode(hdc, TRANSPARENT);
//		EndPaint(hWnd, &ps);
//		break;
	}

	if( iMessage == WM_COMMAND )
		SetFocus(*AVDirector::GetDiector()->GetApplication()->GetHwnd());


	return false;
}