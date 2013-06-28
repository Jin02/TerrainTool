#pragma once

#include "AVQuadTree.h"
#include <math.h>
#include <vector>
#include <string>
#include "AVBase.h"
#include "AVSplatting.h"
#include "SharedTexture.h"
#include "toolTypeDef.h"

using namespace std;

class AVCamera;

typedef struct TILE_INFO
{
	TERRAIN_VERTEX*			pVertex;
	LPDIRECT3DVERTEXBUFFER9	pVB;
	int	nCorner[4];
	int nCenter;
	int	nLodLevel;
	int nLodCrackIndex;
	int	nTriangleNum;

	TILE_INFO(): pVertex(NULL),
				pVB(NULL),
				nCenter(0),
				nLodLevel(0),
				nLodCrackIndex(0),
				nTriangleNum(0)
	{
		int i=0;
		for( i=0; i<4; ++i )
			nCorner[i] = -1;
	}
	~TILE_INFO()
	{
		SAFE_ARRARY_DELETE(pVertex);
		SAFE_RELEASE(pVB);
	}

}TILE_INFO;

struct EDIT_TERRAIN_RANGE
{
	int startX, endX;
	int startZ, endZ;
	EDIT_TERRAIN_RANGE(){startX = endX = startZ = endZ = 0;}
	EDIT_TERRAIN_RANGE(int _startX, int _endX, int _startZ, int _endZ):
	startX(_startX), endX(_endX), startZ(_startZ), endZ(_endZ){}
};

typedef enum EDIT_TERRAIN_OPTION{
	EDIT_TERRAIN_OPTION_NORMAL,
	EDIT_TERRAIN_OPTION_FLAT,
	EDIT_TERRAIN_OPTION_CRATER
};

class AVTerrain : public AVBase
{
private:
	enum CRACK_LOD
	{
		CRACK_LOD_NORMAL,
		CRACK_LOD_LEFT,
		CRACK_LOD_TOP,
		CRACK_LOD_RIGHT,
		CRACK_LOD_BOTTOM
	};

private:
	int								m_nTile;
	int								m_nTotalTiles;
	int								m_nSideVertex;

	//in tile
	int								m_nCell;
	int								m_nTileVertex;
	int								m_nTileIndices;
	int								m_nTileTriangles;


private:
	int								m_nMaxLODLevel;
	int								m_nLODLevelSize;

//	D3DXVECTOR3						m_vfScale;
	float							m_fScale;
	LPDIRECT3DDEVICE9				m_pd3dDevice;
	LPDIRECT3DINDEXBUFFER9			(*m_ppIB)[5];


	TERRAIN_VERTEX					*m_pHeightMap;
	TILE_INFO						*m_pTileInfo;

	std::vector<LPDIRECT3DTEXTURE9> m_vTex;

	std::vector<AVSplatting*>		m_vSplatting;
	std::vector<int>				m_vVisibleTiles;
	std::vector<int>				m_vSplattingTiles;

	AVQuadTree						*m_pSLODTree;
	AVQuadTree						*m_pTerrainTree;

	AVFrustum						*m_pFrustum;
	AVCamera						*m_pCamera;

	PICK_DIR						 m_isPick;
//	bool							 m_bSplattingMode;
	SPLATTING_OPTION				 m_splatOption;
	int								 m_SplattingIndex;

	bool							 m_bLight;
	bool							 m_bFog;
	bool							 m_bWire;
	

public:
	int								m_nBrushSize; //
	BRUSH_TYPE						m_brushType; //
	int								m_nBaseTexID;

public:
	AVTerrain(LPDIRECT3DDEVICE9 pDev, AVFrustum *pFrustum, AVCamera *pCamera);
	~AVTerrain(void);

private:
	void					_Destroy();
	bool					_BuildHeightMap( );

	void					_BaseRender(int index);
	void					_SplattingRender(int index);
	void					_textureSetting(int index, LPDIRECT3DTEXTURE9 tex);
	void					_SetMapInformation(int nTile, int nCell);

	bool					_CreateVertexBuffer(int index, TILE_INFO *pTileInfo);
	bool					_CreateIndexBuffer(int i);
	int						_SearchPositionInTile(D3DXVECTOR3 &vPosition);
	void					_SearchSplattingTiles(D3DXVECTOR3 &vPosition, float size);
	void					_CalcNormalVector(int x[3], int z[3]);

public:
	LPDIRECT3DTEXTURE9		AddTexture( TEXTURE *tex );
	bool					Create(
		float fScale, wchar_t *baseImg,
		int nCell, int nTile, 
		vector<D3DXVECTOR3>*		pvHeightMap	 = NULL, 
		vector<pair<int, BYTE*>>*	pvSplatting  = NULL,
		vector<int>*				pvSplattingVisibleTiles = NULL);
	void					SetLOD(D3DXVECTOR3 &vLook, int index);
	void					SetCrack(int index);

	std::vector<int>*		GetVisibleTiles();
	int						GetTilesNum();
	int						GetCellsNum();
	TERRAIN_VERTEX*			GetHeightMap();
	float					GetHeight(float x, float z);
	bool					Picking(D3DXVECTOR2 &vMouse);

	void					DrawCircleBrush(D3DXVECTOR3 &vPickPos, float size, int count = 25);
	void					DrawSquareBrush(D3DXVECTOR3 &vPickPos, float size, int count = 25);
	void					DrawDotBrush(D3DXVECTOR3 &vPickPos);
	void					EditTerrain( D3DXVECTOR3&vPickPos, BRUSH_TYPE brushType, float size, float editRate, EDIT_TERRAIN_OPTION option = EDIT_TERRAIN_OPTION_NORMAL);
	std::vector<int>		SearchTileArea(D3DXVECTOR3 &vPickPos, float size);

	bool					AddSplattingTexture(int index, int id);
	void					DeleteSplattingTexture(int id);
	bool					SetSplattingTexture(int id);
	void					SetSplattingMode(SPLATTING_OPTION option);// { m_bSplattingMode = b; }
	SPLATTING_OPTION		SplattingMode() { return m_splatOption; }
	void					SetLODLevel(int nLevel) { m_nLODLevelSize = nLevel; }
	void					SetIsFog(bool b) { m_bFog = b; }
	void					SetIsWire(bool b) { m_bWire = b; }
	void					SetIsLight(bool b) { m_bLight = b; }
	int						SizeSplatTextures();

	std::vector<int>&			vSpalttingVisibleTiles() { return m_vSplattingTiles; }
	std::vector<AVSplatting*>&	vSplatting() { return m_vSplatting; }

private:
	int						SearchSplattingTex(LPDIRECT3DTEXTURE9 tex);

public:
	virtual bool			Update();
	virtual bool			Render();
};

