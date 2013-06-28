#pragma once

#include "Information.h"
#include "AVFrustum.h"
#include <vector>

using namespace std;

typedef enum PICK_DIR
{
	PICK_DIR_NONE,
	PICK_DIR_UP,
	PICK_DIR_DOWN
};

class AVQuadTree
{
public:
	enum QUADTREE_DIRECTION{ DIR_TL, DIR_TR, DIR_BL, DIR_BR };
	enum QUADTREE_NEIGHBOR_DIRECTRION{ NGR_LEFT, NGR_RIGHT, NGR_UP, NGR_DOWN };
	
	enum FRUSTUM_TYPE{ 
		FRUSTUM_TYPE_COMPLETE, FRUSTUM_TYPE_COMPLETELY,
		FRUSTUM_TYPE_NO, FRUSTUM_TYPE_UNKNOWN = -1 };

private:
	int					 m_nCorner[4];
	int					 m_nCenter;
	D3DXVECTOR3			 m_vCenter;

	AVQuadTree			*m_pParent;
	
	bool				 m_bCulled;
	float				 m_fRadius;

	AVQuadTree			*m_pChild[4];

public:
	AVQuadTree(AVQuadTree *pParent);
	AVQuadTree(int nSizeXZ);

	~AVQuadTree(void);

private:
	AVQuadTree*  _AddChild(int cornerTL, int cornerTR, int cornerBL, int cornerBR);
	void		 _SetCorner(int cornerTL, int cornerTR, int cornerBL, int cornerBR);
	bool		 _SubDivide();

	int			 _IsInFrustum(TERRAIN_VERTEX *pHeightMap, AVFrustum *pFrustum, int nTile, int nCell);
	int			 _IsInFrustum(TERRAIN_VERTEX *pHeightMap, AVFrustum *pFrustum );

	void		 _FrustumCull(TERRAIN_VERTEX *pHeightMap, AVFrustum *pFrustum, int nTile, int nCell);
	void		 _FrustumCull(TERRAIN_VERTEX *pHeightMap, AVFrustum *pFrustum );

	float		 _GetDistance( D3DXVECTOR3* pv1, D3DXVECTOR3* pv2 );
	void		 _Destroy();

	int			 _SearchVisibleTile(int tileNum, int nTiles, std::vector<int> *pV);

public:
	int			IsMinimum( );

	void		Build( TERRAIN_VERTEX *pHeightMap, int cell, int tile);
	void		Build( TERRAIN_VERTEX *pHeightMap );

	void		GetCorner( int& _0, int& _1, int& _2, int& _3 );
	
	void		Update(std::vector<int> *pV, TERRAIN_VERTEX *pHeightMap, AVFrustum *pFrustum, int &nTile, int &nCell);
	void		Update(TERRAIN_VERTEX *pHeightMap, AVFrustum *pFrustum);

	PICK_DIR	Picking(TERRAIN_VERTEX *pHeightMap, D3DXVECTOR2 &vMouse, int *pPickTL = NULL);
};