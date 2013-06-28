#include "EditDlg.h"
#include "resource.h"
#include <commCtrl.h>
#include "AVDirector.h"
//#include "ToolInformation.h"

static CEditDlg *edtDlg;

CEditDlg::CEditDlg(HINSTANCE hInst, HWND parentHandle)
	:AVDialogBox(hInst, parentHandle, IDD_DLG_MAP, (DLGPROC)CEditDlg::_EditProc)
{
	edtDlg = this;
}

CEditDlg::~CEditDlg(void)
{
}

void CEditDlg::ContentInit()
{
	int nRate = 1;
	int nFrustum = 1000;
	int nLOD = 250;

	_SetRangeTrackBar(IDC_SLIDE_EDITRATE,	1, 20);
	_SetRangeTrackBar(IDC_SLIDE_FRUSTUM,	1, 2000);
	_SetRangeTrackBar(IDC_SLIDE_LOD,		1, 500);

	_SetPosTrackBar(IDC_SLIDE_EDITRATE,		nRate);
	_SetPosTrackBar(IDC_SLIDE_FRUSTUM,		nFrustum);
	_SetPosTrackBar(IDC_SLIDE_LOD,			nLOD);

	_SetEdtValue(IDC_EDIT_FRUSTUM, nFrustum);
	_SetEdtValue(IDC_EDIT_EDITRATE, nRate);
	_SetEdtValue(IDC_EDIT_LOD, nLOD);

	_SetCheckBox(IDC_RADIO_BULGING_DOWN, false);
	_SetCheckBox(IDC_RADIO_BULGING_UP, false);
	_SetCheckBox(IDC_RADIO_FLAT_DOWN, false);
	_SetCheckBox(IDC_RADIO_FLAT_UP, false);
	_SetCheckBox(IDC_RADIO_CREATER_DOWN, false);
	_SetCheckBox(IDC_RADIO_CREATER_UP, false);
}

void CEditDlg::_EditBox(UINT msg, UINT id)
{
	if(EN_CHANGE != msg) return;

//	wchar_t txt[30];
	int value = GetDlgItemInt(m_hWnd, id, NULL, true);
	float w, h, dist;
	Scn_MapTool *scn = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene());

	if(value<1)
	{
		value = 1;
		_SetEdtValue(id, value);
		MessageBox(m_parentHandle, L"최소 1 이상이여야 합니다", L"경고", MB_OK);
	}
	
	if( value > 2000 && id == IDC_EDIT_FRUSTUM )
	{
		value = 2000;
		_SetEdtValue(id, value);
		MessageBox(m_parentHandle, L"최대값은 2천을 넘을수가 없습니다.", L"경고", MB_OK);
	}

	switch(id)
	{
	case IDC_EDIT_EDITRATE:
		_SetPosTrackBar(IDC_SLIDE_EDITRATE, value);
//		CToolInformation::GetToolInfo()->nEditRate = value;
		scn->SetEditRate(value);
		break;
	case IDC_EDIT_FRUSTUM:
		_SetPosTrackBar(IDC_SLIDE_FRUSTUM, value);

		w = AVDirector::GetDiector()->GetApplication()->GetWindowSize().w;
		h = AVDirector::GetDiector()->GetApplication()->GetWindowSize().h;
		dist = static_cast<float>(value);

		AVDirector::GetDiector()->SetMatrixProjection(D3DX_PI/4, w/h, 1.f, dist);
		AVDirector::GetDiector()->SetFog(dist - 100.f, dist - 50.f);

		break;
	case IDC_EDIT_LOD:	
		_SetPosTrackBar(IDC_SLIDE_LOD, value);
		//AVDirector::GetDiector()->nLODLevel = value;
		Scn_MapTool * scn = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene());
		scn->SetLODLevel(value);

		break;
	}
}

void CEditDlg::_SlideBar(HWND hWnd)
{
	Scn_MapTool *scn = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene());
	int pos = SendMessage(hWnd, TBM_GETPOS, 0, 0);

	if( hWnd == GetDlgItem(m_hWnd, IDC_SLIDE_EDITRATE) )
	{
		_SetEdtValue(IDC_EDIT_EDITRATE, pos);
//		CToolInformation::GetToolInfo()->nEditRate = pos;
		scn->SetEditRate(pos);
	}
	else if ( hWnd == GetDlgItem(m_hWnd, IDC_SLIDE_FRUSTUM) )
	{
		_SetEdtValue(IDC_EDIT_FRUSTUM, pos);
		//AVDirector::GetDiector()->nFrustum = pos;
		float w = AVDirector::GetDiector()->GetApplication()->GetWindowSize().w;
		float h = AVDirector::GetDiector()->GetApplication()->GetWindowSize().h;
		float dist = static_cast<float>(pos);

		AVDirector::GetDiector()->SetMatrixProjection(D3DX_PI/4, w/h, 1.f, dist);
		AVDirector::GetDiector()->SetFog(dist - 100.f, dist - 50.f);
	}
	else
	{
		_SetEdtValue(IDC_EDIT_LOD, pos);
		scn->SetLODLevel(pos);
	}
}

bool CALLBACK CEditDlg::_EditProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
//	HDC			hdc;
//	PAINTSTRUCT ps;
	HBRUSH		hBrush=CreateSolidBrush(RGB(255,228,213)); 
	Scn_MapTool *scn = dynamic_cast<Scn_MapTool*>(AVDirector::GetDiector()->GetScene());

	START_ENABLE(iMessage);
	
	switch(iMessage)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_RADIO_BULGING_DOWN:
			//CToolInformation::GetToolInfo()->editMode = 	TERRAIN_EDIT_TYPE_BULGING_DOWN;
			scn->SetEditType(TERRAIN_EDIT_TYPE_BULGING_DOWN);
			break;

		case IDC_RADIO_BULGING_UP:
			//CToolInformation::GetToolInfo()->editMode = 	TERRAIN_EDIT_TYPE_BULGING_UP;
			scn->SetEditType(TERRAIN_EDIT_TYPE_BULGING_UP);
			break;

		case IDC_RADIO_FLAT_DOWN:
			//CToolInformation::GetToolInfo()->editMode = 	TERRAIN_EDIT_TYPE_FLAT_DOWN;
			scn->SetEditType(TERRAIN_EDIT_TYPE_FLAT_DOWN);
			break;

		case IDC_RADIO_FLAT_UP:
//			CToolInformation::GetToolInfo()->editMode = 	TERRAIN_EDIT_TYPE_FLAT_UP;
			scn->SetEditType(TERRAIN_EDIT_TYPE_FLAT_UP);
			break;

		case IDC_RADIO_CREATER_DOWN:
//			CToolInformation::GetToolInfo()->editMode = 	TERRAIN_EDIT_TYPE_CREATER_DOWN;
			scn->SetEditType(TERRAIN_EDIT_TYPE_CREATER_DOWN);
			break;

		case IDC_RADIO_CREATER_UP:
//			CToolInformation::GetToolInfo()->editMode = 	TERRAIN_EDIT_TYPE_CREATER_UP;
			scn->SetEditType(TERRAIN_EDIT_TYPE_CREATER_UP);
			break;

		case IDC_EDIT_LOD:
		case IDC_EDIT_FRUSTUM:
		case IDC_EDIT_EDITRATE: 
			edtDlg->_EditBox(HIWORD(wParam), LOWORD(wParam));		break;
		}
		break;
	case WM_HSCROLL:
		edtDlg->_SlideBar(reinterpret_cast<HWND>(lParam));
		break;
	}

	if( iMessage == WM_COMMAND )
		SetFocus(*AVDirector::GetDiector()->GetApplication()->GetHwnd());


	return 0;
}