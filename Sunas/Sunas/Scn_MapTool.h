#pragma once

#include "AVScene.h"
#include "AVTerrain.h"
#include "AVString.h"
#include "toolTypeDef.h"

class Scn_MapTool :
	public AVScene
{
private:
	LPDIRECT3DDEVICE9	 m_device;
	AVTerrain			*m_pTerrain;
	AVFrustum			*m_pFrustum;

	AVString			m_strFPS;
	AVString			m_strPick;
	AVString			m_strCamera;
	AVString			m_strKey;

	int					m_nCell;
	int					m_nTile;
	int					m_nScale;
	int					m_nCameraSpeed;
	std::vector<int>	m_vSplattingVisible;
	vector<std::pair<int, BYTE*>>	m_vSplatting;
	vector<D3DXVECTOR3>	m_vHeightMap;
	bool				m_isLoad;

	int					m_nEditRate;
	TERRAIN_EDIT_TYPE	m_editType;
	wchar_t				*m_texPath;

	SPLATTING_OPTION	m_splatOption;



public:
	Scn_MapTool(void* data = NULL);
	~Scn_MapTool(void);

public:
	virtual void Init();
	virtual void Loop();
	virtual	void End();
	virtual void Mouse(D3DXVECTOR2 &vPosition, int Left, int Right);
	virtual void KeyBoard(bool *push, bool *up);

public:
	void SetLODLevel(int nLevel);
//	void SetRenderOption(bool bLight, bool bFog, bool bWire);
	void SetIsLight(bool b) { m_pTerrain->SetIsLight(b); }
	void SetIsFog(bool b) { m_pTerrain->SetIsFog(b); }
	void SetIsWire(bool b) { m_pTerrain->SetIsWire(b); }
	void SetCameraSpeed(int nSpeed) { m_nCameraSpeed = nSpeed; }

	void SetEditType(TERRAIN_EDIT_TYPE type) { m_editType = type; }
	void SetEditRate(int nRate);// { m_nEditRate = nRate; }
	void SetBrushSize(int nSize);// { m_nBrushSize = nSize; }
	void SetBrushType(BRUSH_TYPE type);// { m_brushType = type; }
	void SetSplatOption(SPLATTING_OPTION option);
	//std::vector<int>& vSplattingTexID() { return m_vSplattingTexID; }
	bool AddSplattingTex(int index, int id);
	void DeleteSplattingTex(int id);
	void SelSplatTex(int id);
	int  SizeSplatTextures();
	int  baseTexID();

	void loadTerrainFile(wchar_t *path);
	void saveTerrainFile(wchar_t *path);
};