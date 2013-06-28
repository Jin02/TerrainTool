#include "Scn_MapTool.h"
#include "AVDirector.h"
#include "AVCamera.h"
#include "AVPicking.h"

Scn_MapTool::Scn_MapTool(void* data)
{
	MapInfo	*pInfo;
	pInfo = reinterpret_cast<MAPINFO*>(data);
	m_nCell = pInfo->nCell;
	m_nTile = pInfo->nTile;
	m_nScale = pInfo->nScale;
	m_texPath = pInfo->baseImgPath;
	m_isLoad = false;

	if(pInfo->pvHeightMap)
	{
		for(std::vector<int>::iterator iter = pInfo->pvSplattingVisible->begin();	iter != pInfo->pvSplattingVisible->end();	++iter)
			m_vSplattingVisible.push_back((*iter));

		for(std::vector<std::pair<int, BYTE*>>::iterator iter = pInfo->pvSplatting->begin(); iter != pInfo->pvSplatting->end(); ++iter)
			m_vSplatting.push_back((*iter));

		for(std::vector<D3DXVECTOR3>::iterator iter = pInfo->pvHeightMap->begin(); iter != pInfo->pvHeightMap->end(); ++iter)
			m_vHeightMap.push_back((*iter));

		m_isLoad = true;
	}

	m_pTerrain = NULL;
	m_nCameraSpeed = 1;

	m_editType = TERRAIN_EDIT_TYPE_NONE;
	m_nEditRate = 1;
	m_splatOption = SPLATTING_OPTION_NONE;
}

Scn_MapTool::~Scn_MapTool(void)
{
	SAFE_DELETE(m_pTerrain);
	SAFE_DELETE(m_pFrustum);

	AVCamera::GetCamera()->deleteCamera();
	AVPicking::GetPicking(m_device)->deletePickingMgr();
}

void Scn_MapTool::Init()
{
	m_device = AVDirector::GetDiector()->GetApplication()->GetD3DDevice();

	m_pFrustum = new AVFrustum;	
	m_pTerrain = new AVTerrain(m_device, m_pFrustum, AVCamera::GetCamera());
	//m_pTerrain->Create(m_device, 
	//	m_nScale, 
	//	NULL,
	//	m_texPath,
	//	m_pFrustum, AVCamera::GetCamera(),
	//	m_nCell,
	//	m_nTile);
	for(int i=0; i<TILE_TEXTURE_NUM; i++)
	{
		TEXTURE *tex = SharedTexture::GetSharedTexture()->GetTexture(i);
		m_pTerrain->AddTexture(tex);
		//중복이고 뭐고 일단 넣자; 코드가 좀 꼬인거 같다
	}

	if(m_isLoad == false)
		m_pTerrain->Create(m_nScale, m_texPath, m_nCell, m_nTile);
	else
	{
		m_pTerrain->Create(m_nScale, m_texPath, m_nCell, m_nTile, &m_vHeightMap, &m_vSplatting, &m_vSplattingVisible);
		m_vSplattingVisible.clear();
		m_vHeightMap.clear();
		vector<std::pair<int, BYTE*>>::iterator iter;

		for(iter = m_vSplatting.begin(); iter != m_vSplatting.end(); ++iter)
		{
			delete[] iter->second;
		}
		m_vSplatting.clear();
	}


	addObject(m_pTerrain);

	AVCamera::GetCamera()->setCameraType(AVCamera::CAMERA_TYPE_AIRCRAFT);	
	//	AVCamera::GetCamera()->setPosition(D3DXVECTOR3(-4.f, 10.f, -23.f));
	//맵의 한 가운데로.
	//x = mapX / 2, y = MapX / 2, z = MapZ / 2;

	int mapSize = m_nCell * m_nTile * m_nScale;
	D3DXVECTOR3 pos;
	pos.x =  (float)mapSize / 2.f;
	pos.y =  (float)mapSize / 10.f;
	pos.z = -(float)mapSize / 2.f - (float)mapSize / 3.;

	AVCamera::GetCamera()->setPosition(pos);

	AVCamera::GetCamera()->pitch(D3DX_PI/180.f * 30.f);

	m_strFPS.CreateDefault(m_device);
	m_strFPS.SetRect(AVRECT(0,0,130,30));

	m_strPick.CreateDefault(m_device);
	m_strPick.SetRect(AVRECT(0,35,400,30));

	m_strCamera.CreateDefault(m_device);
	m_strCamera.SetRect(AVRECT(0,60,400,30));

	m_strKey.CreateDefault(m_device);
	m_strKey.SetRect(AVRECT(0,80, 400, 120));
	m_strKey.SetText(L"W = Up, S = Down \n A = Left, D = Right \n R = Look up, T = Look Down \n F = Left Rotate, G = Right Rotate");

	addObject(&m_strFPS);
	addObject(&m_strPick);
	addObject(&m_strCamera);
	addObject(&m_strKey);
}

void Scn_MapTool::Loop()
{
	D3DXMATRIX matView, matProj;

	AVCamera::GetCamera()->getViewMatrix(&matView);
	matProj = *AVDirector::GetDiector()->GetMatrixProjection();
	//m_device->GetTransform(D3DTS_PROJECTION, &matProj);

	D3DXMATRIXA16 m;
	m = matView * matProj;
	m_pFrustum->Make(&m);

	m_device->SetTransform(D3DTS_VIEW, &matView);

	float fps = AVDirector::GetDiector()->GetFPS();
	wchar_t fpsContent[20];

	wsprintf(fpsContent, L"FPS : %d", (int)fps);
	m_strFPS.SetText(fpsContent);
}

void Scn_MapTool::End()
{
}

void Scn_MapTool::Mouse(D3DXVECTOR2 &vPosition, int Left, int Right)
{
	static LPDIRECT3DDEVICE9 sDevice = AVDirector::GetDiector()->GetApplication()->GetD3DDevice();

	D3DXVECTOR3 vPicking = AVPicking::GetPicking(sDevice)->GetPickPos();

	if( m_pTerrain->Picking(vPosition) == false )
		return;

	wchar_t text[80];
	wsprintf(text, L"Picking Pos x = %d / y = %d / z = %d", (int)vPicking.x, (int)vPicking.y, (int)vPicking.z);
	m_strPick.SetText(text);

	if( Left != AV_MOUSE_DOWN)
	{
		m_pTerrain->SetSplattingMode(SPLATTING_OPTION_NONE);
		return;
	}

	if(m_splatOption == SPLATTING_OPTION_NONE)
	{

		float rate = (float)m_nEditRate;

		if( m_editType % 2 == 0 )
			rate *= -1;

		switch(m_editType)
		{
		case TERRAIN_EDIT_TYPE_BULGING_DOWN:
		case TERRAIN_EDIT_TYPE_BULGING_UP:
			m_pTerrain->EditTerrain(vPicking, m_pTerrain->m_brushType, m_pTerrain->m_nBrushSize,
				rate, EDIT_TERRAIN_OPTION_NORMAL);
			break;
		case TERRAIN_EDIT_TYPE_FLAT_DOWN:
		case TERRAIN_EDIT_TYPE_FLAT_UP:
			m_pTerrain->EditTerrain(vPicking, m_pTerrain->m_brushType, m_pTerrain->m_nBrushSize,
				rate, EDIT_TERRAIN_OPTION_FLAT);
			break;
		case TERRAIN_EDIT_TYPE_CREATER_DOWN:
		case TERRAIN_EDIT_TYPE_CREATER_UP:
			m_pTerrain->EditTerrain(vPicking, m_pTerrain->m_brushType, m_pTerrain->m_nBrushSize,
				rate, EDIT_TERRAIN_OPTION_CRATER);
			break;
		}
	}
	else
	{
		//		m_pTerrain->SetSplattingMode(true);
		//		m_pTerrain-
		m_pTerrain->SetSplattingMode(m_splatOption);
	}
}

void Scn_MapTool::KeyBoard(bool *push, bool *up)
{
	AVCamera *camera = AVCamera::GetCamera();
	float speed = 1.3f * static_cast<float>(m_nCameraSpeed);

	if(push['W'])
		camera->walk(speed);
	if(push['S'])
	{
		camera->walk(-speed);
	}
	if(push['A'])
		camera->strafe(-speed);
	if(push['D'])
		camera->strafe(speed);
	if(push['T'])
	{
		camera->pitch(D3DX_PI/180.f);
	}
	if(push['R'])
	{
		camera->pitch(-D3DX_PI/180.f);
	}
	if(push['G'])
		camera->yaw(D3DX_PI/180.f);
	if(push['F'])
		camera->yaw(-D3DX_PI/180.f);

	wchar_t str[80];
	D3DXVECTOR3 v = camera->getPosition();
	wsprintf(str, L"Camera x = %d, y = %d, z = %d", static_cast<int>(v.x), static_cast<int>(v.y), static_cast<int>(v.z));
	m_strCamera.SetText(str);
}

void Scn_MapTool::SetLODLevel(int nLevel)
{
	m_pTerrain->SetLODLevel(nLevel);
}

void Scn_MapTool::SetEditRate(int nRate)
{
	m_nEditRate = nRate;
}

void Scn_MapTool::SetBrushSize(int nSize)
{
	m_pTerrain->m_nBrushSize = nSize;
}

void Scn_MapTool::SetBrushType(BRUSH_TYPE type)
{
	m_pTerrain->m_brushType = type;
}

void Scn_MapTool::SetSplatOption(SPLATTING_OPTION option)
{
	m_splatOption = option;
	//bool b = false;

	//if( m_splatOption != SPLATTING_OPTION_NONE )
	//	b = true;

	//	m_pTerrain->SetSplattingMode(b);
}

bool Scn_MapTool::AddSplattingTex(int index, int id)
{
	//인덱스는 현재 등록된 텍스쳐에서 뜻하는 것이며
	//id는 스플래팅하는 텍스쳐를 고를수 있는 아이디다
	//인덱스는 올리스트에서 순서대로랑 똑같으니까 그거 넣어주면 되고
	//id는 오른쪽 리스트에서 하면될거야.
	return m_pTerrain->AddSplattingTexture(index, id);
}

void Scn_MapTool::DeleteSplattingTex(int id)
{
	m_pTerrain->DeleteSplattingTexture(id);
}

void Scn_MapTool::SelSplatTex(int id)
{
	if(m_pTerrain->SetSplattingTexture(id) == false)
	{
		int a = 5;
		a=3;
	}
	//	m_pTerrain->SetSplattingMode
}

int Scn_MapTool::SizeSplatTextures()
{
	return m_pTerrain->SizeSplatTextures();
}

int  Scn_MapTool::baseTexID()
{
	return m_pTerrain->m_nBaseTexID;
}

void Scn_MapTool::saveTerrainFile(wchar_t *path)
{
	FILE *fp = _wfopen(path, L"wb");
	int nSideVertex = m_nTile * m_nCell + 1;
	int size;

	fwrite(&m_nTile, sizeof(int), 1, fp);
	fwrite(&m_nCell, sizeof(int), 1, fp);
	fwrite(&m_nScale, sizeof(int), 1, fp);
	fwrite(&m_pTerrain->m_nBaseTexID, sizeof(int), 1, fp);
	//fwrite(m_pTerrain->GetHeightMap(), sizeof(TERRAIN_VERTEX), nSideVertex * nSideVertex -1, fp);

	size = nSideVertex * nSideVertex -1;

	for(int i=0; i<size; i++)
		fwrite(&m_pTerrain->GetHeightMap()[i].p, sizeof(D3DXVECTOR3), 1, fp);

	size = m_pTerrain->vSplatting().size();
	fwrite(&size, sizeof(int), 1, fp);

	for(int i=0; i<size; i++)
	{
		int id = m_pTerrain->vSplatting()[i]->texID();
		fwrite(&id, sizeof(int), 1, fp);
		m_pTerrain->vSplatting()[i]->save(fp);
	}

	size = m_pTerrain->vSpalttingVisibleTiles().size();
	fwrite(&size, sizeof(int), 1, fp);

	for(int i=0; i<size; i++)
		fwrite(&m_pTerrain->vSpalttingVisibleTiles()[i], sizeof(int), 1, fp);

	fclose(fp);
}