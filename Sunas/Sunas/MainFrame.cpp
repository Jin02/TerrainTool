#include "MainFrame.h"
#include "SharedTexture.h"

static CMainFrame *mainFrame;

CMainFrame::CMainFrame(AVRECT &rect, HINSTANCE hInst)
	:AVWindow(rect, hInst, false, L"TerrainTool", NULL,
	CMainFrame::MainWndProc, MAKEINTRESOURCE(IDR_MENU1))
{
	m_hIsnt = hInst;
	mainFrame = this;
	m_bFirst = false;
	m_bisLoad = false;
}

CMainFrame::~CMainFrame(void)
{
	SAFE_DELETE(m_Tool);
	SharedTexture::GetSharedTexture()->deleteAllTexture();
	SharedTexture::GetSharedTexture()->Delete();
}

void  CMainFrame::_LoadTextures()
{
	SharedTexture::GetSharedTexture()->addFile(L"Resource/grass1.bmp",		0);
	SharedTexture::GetSharedTexture()->addFile(L"Resource/grass2.bmp",		1);
	SharedTexture::GetSharedTexture()->addFile(L"Resource/grayStone.bmp",	2);
	SharedTexture::GetSharedTexture()->addFile(L"Resource/sand1.bmp",		3);
	SharedTexture::GetSharedTexture()->addFile(L"Resource/sand2.bmp",		4);
	SharedTexture::GetSharedTexture()->addFile(L"Resource/sand3.bmp",		5);
	SharedTexture::GetSharedTexture()->addFile(L"Resource/tile.bmp",			6);
	SharedTexture::GetSharedTexture()->addFile(L"Resource/tileStone.bmp",	7);
}

void CMainFrame::Run()
{
	m_director				= AVDirector::GetDiector();
	bool		bFrameCheck = m_director->GetFrameCheck();

	Create();
	m_director->RunApplication(1000, m_hIsnt, m_hWnd, true, AVRECT(0, 0, 700, 700), L"3D");
	_LoadTextures();

	m_Tool = new CTerrainTool(m_hIsnt, m_hWnd);
	m_Tool->Create();

	Show();
	m_Tool->Show(700,0,300,700);
	SetFocus(*m_director->GetApplication()->GetHwnd());

	MSG msg;
	ZeroMemory( &msg, sizeof( msg ) );

	while( msg.message != WM_QUIT )
	{
		if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			m_director->RunToInnerSystem();
			m_director->tickTime();

			if(bFrameCheck)
				m_director->calculateFPS();

			if( m_bFirst == false && (m_director->GetState() == AV_STATE_LOOP))
			{
				m_Tool->ContentInit();

				if(m_bisLoad)
				{
					for(int i=0; i<m_splatTexID.size(); i++)
						m_Tool->SplatDlgTextureAdd(m_splatTexID[i]);
				}
				m_bFirst = true;
			}
		}
	}

	m_Tool->Delete();
	AVWindow::Destroy();
}

void CMainFrame::_MenuFileNew()
{
	CNewMap newMap(m_hIsnt, m_hWnd);
	newMap.DoModal();

	if(newMap.GetisCreate() == false)			return;
	
	MAPINFO info;
	info.nCell  = newMap.GetCellSize();
	info.nTile  = newMap.GetTileSize();
	info.nScale = newMap.GetScale();
	info.baseImgPath = SharedTexture::GetSharedTexture()->GetTexture(newMap.GetSelTexID())->path;

	void *data = reinterpret_cast<void*>(&info);
	m_director->Start(dynamic_cast<AVScene*>(new Scn_MapTool(data)));
	m_bFirst = false;
	m_bisLoad = false;

	/* 자 이제 맵 생성이야 */
}

void CMainFrame::_MenuFileSave(HWND hWnd)
{
	wchar_t name[256] = {0, };
	Scn_MapTool *scn = dynamic_cast<Scn_MapTool*>(m_director->GetScene());
	if(scn == NULL)
	{
		MessageBox(m_hWnd, L"아무것도 생성하시지 않으셨습니다", L"오류", MB_OK);
		return;
	}
	
	OPENFILENAME ofn;
	memset( &ofn, 0, sizeof(OPENFILENAME) );

	ofn.lpstrDefExt		= L"terrain";
	ofn.lpstrFilter		= L"terrain 파일\0*.terrain";

	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= hWnd;

	ofn.lpstrTitle		= NULL;

	ofn.lpstrFile		= m_strPath;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= name;
	ofn.nMaxFileTitle	= MAX_PATH;
	ofn.hInstance		= m_hIsnt;

	if( GetSaveFileName(&ofn) != 0 )
	{
		
		scn->saveTerrainFile(m_strPath);
	}
}

void CMainFrame::_MenuFileLoad(HWND hWnd)
{
	wchar_t name[256] = {0, };
	Scn_MapTool *scn = dynamic_cast<Scn_MapTool*>(m_director->GetScene());
	OPENFILENAME ofn;
	memset( &ofn, 0, sizeof(OPENFILENAME) );

	ofn.lpstrDefExt		= L"terrain";
	ofn.lpstrFilter		= L"terrain 파일 .terrain\0*.terrain";
//	mp3 파일 .mp3\0*.mp3

	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= hWnd;

	ofn.lpstrTitle		= NULL;

	ofn.lpstrFile		= m_strPath;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= name;
	ofn.nMaxFileTitle	= MAX_PATH;
	ofn.hInstance		= m_hIsnt;

	if(GetOpenFileName(&ofn) != 0)
	{
		loadTerrainFile(m_strPath);
		m_bisLoad = true;
	}
}

void CMainFrame::_MenuExit()
{

}

LRESULT WINAPI CMainFrame::MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static WNDPROC dirProc = mainFrame->m_director->WndProc;

	switch(msg)
	{
	case WM_KEYUP:
	case WM_KEYDOWN:
		dirProc(hWnd, msg, wParam, lParam);
		break;

	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
		case ID_MENU_FILE_NEW:
			mainFrame->_MenuFileNew();
			break;
		case ID_MENU_FILE_LOAD:
			mainFrame->_MenuFileLoad(hWnd);
			break;
		case ID_MENU_FILE_SAVE:
			mainFrame->_MenuFileSave(hWnd);
			break;
		case ID_MENU_FILE_EXIT:
			mainFrame->_MenuExit();
			break;
		case ID_ABOUT_ABOUT:
			break;
		}

		break;

	case WM_DESTROY:
		mainFrame->m_director->GetDiector()->deleteDirector();
		break;

	case WM_PAINT:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


void CMainFrame::loadTerrainFile(wchar_t *path)
{
	//생각해보니까 이거 좌표 저장도 y만 하면되잖아? 뭐하러 x,z 다 했지;
	FILE *fp = _wfopen(path, L"rb");

	vector<int>					vSplattingVisible;
	vector<pair<int, BYTE*>>	vSplatting;
	vector<D3DXVECTOR3>			vHeightMap;
	int nTile, nCell, nScale;
	
	fread(&nTile,  sizeof(int),  1, fp);
	fread(&nCell,  sizeof(int),  1, fp);
	fread(&nScale, sizeof(int),  1, fp);
	
	int baseImgID;
	fread(&baseImgID, sizeof(int), 1, fp);

	int size = (nTile * nCell + 1) * (nTile * nCell + 1) - 1;

	for(int i=0; i<size; ++i)
	{
		D3DXVECTOR3 pos;
		fread(&pos, sizeof(D3DXVECTOR3), 1, fp);
		vHeightMap.push_back(pos);
	}

	int nSplatting;
	fread(&nSplatting, sizeof(int), 1, fp);

	for(int i=0; i<nSplatting; ++i)
	{
		int id;
		fread(&id, sizeof(int), 1, fp);

		BYTE *bits = new BYTE[256*256];
		fread(bits, sizeof(BYTE), 256*256, fp);

		pair<int, BYTE*> pair;
		pair.first = id;
		pair.second = bits;
		vSplatting.push_back(pair);
		m_splatTexID.push_back(id);
	}

	int nSplattingVisible;
	fread(&nSplattingVisible, sizeof(int), 1, fp);

	for(int i=0; i<nSplattingVisible; ++i)
	{
		int n;
		fread(&n, sizeof(int), 1, fp);
		vSplattingVisible.push_back(n);
	}

	fclose(fp);

	MAPINFO info;
	info.nCell  = nCell;
	info.nTile  = nTile;
	info.nScale = nScale;
	info.pvHeightMap = &vHeightMap;
	info.pvSplatting = &vSplatting;
	info.pvSplattingVisible = &vSplattingVisible;
	info.baseImgPath = SharedTexture::GetSharedTexture()->GetTexture(baseImgID)->path;


	void *data = reinterpret_cast<void*>(&info);
	m_director->Start(dynamic_cast<AVScene*>(new Scn_MapTool(data)));
	m_bFirst = false;
//	m_Tool-
}
