#include "AVQuadTree.h"
#include <d3d9.h>
#include <d3dx9.h>
#include "AVPicking.h"
#include "AVDirector.h"

AVQuadTree::AVQuadTree(AVQuadTree *pParent)
{
	int		i;
	m_pParent = pParent;
	m_nCenter = 0;
	for( i = 0 ; i < 4 ; i++ )
	{
		m_pChild[i] = NULL;
		m_nCorner[i] = 0;
	}

	m_bCulled = false;
	m_fRadius = 0.0f;
}

AVQuadTree::AVQuadTree(int nSizeXZ)
{
	//First, Root Node

	m_nCorner[DIR_TL] = 0;
	m_nCorner[DIR_TR] = nSizeXZ;
	m_nCorner[DIR_BL] = (nSizeXZ+1) * nSizeXZ;
	m_nCorner[DIR_BR] = (nSizeXZ+1) * (nSizeXZ+1) -1;
	m_nCenter = (m_nCorner[DIR_TL] + m_nCorner[DIR_TR] + m_nCorner[DIR_BL] + m_nCorner[DIR_BR]) / 4;

	m_bCulled = false;
	m_fRadius = 0.f; // Length(br - tl)/2.f;
}

AVQuadTree::~AVQuadTree(void)
{
	_Destroy();
}

AVQuadTree* AVQuadTree::_AddChild(int cornerTL, int cornerTR, int cornerBL, int cornerBR)
{
	AVQuadTree *child = new AVQuadTree(this);
	child->_SetCorner(cornerTL, cornerTR, cornerBL, cornerBR);

	return child;
}

void AVQuadTree::_SetCorner(int cornerTL, int cornerTR, int cornerBL, int cornerBR)
{
	m_nCorner[DIR_TL] = cornerTL;
	m_nCorner[DIR_TR] = cornerTR;
	m_nCorner[DIR_BL] = cornerBL;
	m_nCorner[DIR_BR] = cornerBR;

	m_nCenter = (cornerTL+cornerTR+cornerBL+cornerBR)/4;
}

bool AVQuadTree::_SubDivide()
{
	int topCenter, leftCenter, rightCenter, bottomCenter, realCenter;

	topCenter    = (m_nCorner[DIR_TL] + m_nCorner[DIR_TR]) / 2;
	leftCenter   = (m_nCorner[DIR_TL] + m_nCorner[DIR_BL]) / 2;
	rightCenter  = (m_nCorner[DIR_TR] + m_nCorner[DIR_BR]) / 2;
	bottomCenter = (m_nCorner[DIR_BL] + m_nCorner[DIR_BR]) / 2;
	realCenter	 = (m_nCorner[DIR_TL] + m_nCorner[DIR_TR] + m_nCorner[DIR_BL] + m_nCorner[DIR_BR]) / 4;

	if( (m_nCorner[DIR_TR] - m_nCorner[DIR_TL]) <= 1 )
		return false;

	m_pChild[DIR_TL] = _AddChild(m_nCorner[DIR_TL], topCenter, leftCenter, realCenter);
	m_pChild[DIR_TR] = _AddChild(topCenter, m_nCorner[DIR_TR], realCenter, rightCenter);
	m_pChild[DIR_BL] = _AddChild(leftCenter, realCenter, m_nCorner[DIR_BL], bottomCenter);
	m_pChild[DIR_BR] = _AddChild(realCenter, rightCenter, bottomCenter, m_nCorner[DIR_BR]);

	return true;
}

int AVQuadTree::_IsInFrustum(TERRAIN_VERTEX *pHeightMap, AVFrustum *pFrustum, int nTile, int nCell)
{
	bool is[4];
	bool isInSphere;

	int cell = nCell;
	int tiles = nTile + 1;
	int vertex = nCell * nTile +1; //same sideVertex in terrain

	if( IsMinimum() == false )
	{
		int center =  ((m_nCenter/tiles) * cell * vertex)
					+ ((m_nCenter%tiles) * cell);

		if( (isInSphere = pFrustum->IsInSphere(&pHeightMap[center].p ,m_fRadius)) == false )
			return FRUSTUM_TYPE_NO;
	}

	for(int i=0; i<4; i++)
	{
		int dir   = ((m_nCorner[i]/tiles) * cell * vertex) 
			      + ((m_nCorner[i]%tiles) * cell);

		is[i] = pFrustum->IsIn(&pHeightMap[dir].p);
	}

	if( (is[0] + is[1] + is[2] + is[3]) == 4 )
		return FRUSTUM_TYPE_COMPLETE;

	return FRUSTUM_TYPE_COMPLETELY;
}

int AVQuadTree::_IsInFrustum(TERRAIN_VERTEX *pHeightMap, AVFrustum *pFrustum )
{
	bool is[4];
	bool isInSphere;

	if(IsMinimum() == false)
	if( (isInSphere = pFrustum->IsInSphere(&pHeightMap[m_nCenter].p ,m_fRadius)) == false )
			return FRUSTUM_TYPE_NO;

	for(int i=0; i<4; i++)
		is[i] = pFrustum->IsIn(&pHeightMap[m_nCorner[i]].p);

	if( (is[0] + is[1] + is[2] + is[3]) == 4 )
		return FRUSTUM_TYPE_COMPLETE;

	return FRUSTUM_TYPE_COMPLETELY;

}

void AVQuadTree::_FrustumCull(TERRAIN_VERTEX *pHeightMap, AVFrustum *pFrustum, int nTile, int nCell)
{
	int result = _IsInFrustum(pHeightMap, pFrustum, nTile, nCell);

	if(result == FRUSTUM_TYPE_COMPLETE)
	{
		m_bCulled = false;
		return;
	}
	else if(result == FRUSTUM_TYPE_COMPLETELY)
		m_bCulled = false;
	else // FRUSTUM_TYPE_NO
	{ m_bCulled = true; return; }

	for(int i=0; i<4; i++)
		if(m_pChild[i])
			m_pChild[i]->_FrustumCull(pHeightMap, pFrustum, nTile, nCell);
}

void AVQuadTree::_FrustumCull(TERRAIN_VERTEX *pHeightMap, AVFrustum *pFrustum )
{
	int result = _IsInFrustum(pHeightMap, pFrustum);

	if(result == FRUSTUM_TYPE_COMPLETE)
	{
		m_bCulled = false;
		return;
	}
	else if(result == FRUSTUM_TYPE_COMPLETELY)
		m_bCulled = false;
	else // FRUSTUM_TYPE_NO
	{ m_bCulled = true; return; }

	for(int i=0; i<4; i++)
		if(m_pChild[i])
			m_pChild[i]->_FrustumCull(pHeightMap, pFrustum);
}

void AVQuadTree::_Destroy()
{
	for(int i=0; i<4; i++)
		SAFE_DELETE(m_pChild[i]);
}

int AVQuadTree::IsMinimum( )
{
	return ((m_nCorner[DIR_TR] - m_nCorner[DIR_TL]) <= 1);	
}

void AVQuadTree::Build(TERRAIN_VERTEX *pHeightMap, int cell, int tile)
{
	int tiles  = tile + 1;
	int vertex = cell * tile + 1;
		
	int tl = ((m_nCorner[DIR_TL]/tiles) * cell * vertex) 
		+ ((m_nCorner[DIR_TL]%tiles) * cell);
	int br = ((m_nCorner[DIR_BR]/tiles) * cell * vertex)
		+ ((m_nCorner[DIR_BR]%tiles) * cell);

	//좌상단, 우하단을 뺌
	D3DXVECTOR3 v = pHeightMap[tl].p - pHeightMap[br].p;

	//경계구의 반지름
	m_fRadius = D3DXVec3Length(&v) / 2.f;
	m_vCenter = (pHeightMap[tl].p + pHeightMap[br].p) / 2.f;

	//최 하단 노드의 반지름, 중심점 셋팅이 전혀 되질 않아.
	//뭐.. 안되면 미리 계산 없애버리고 만들어버리는게 낫기야 하겠다

	//여기서는 타일별이 아닌, 각각 한칸을 기준으로 잡고 나눈다.

	if(_SubDivide() == false)
		return;

	for(int i=0; i<4; i++)
		m_pChild[i]->Build(pHeightMap, cell, tile);
}

void AVQuadTree::Build( TERRAIN_VERTEX *pHeightMap )
{
	//좌상단, 우하단을 뺌
	D3DXVECTOR3 v = pHeightMap[m_nCorner[DIR_TL]].p - pHeightMap[m_nCorner[DIR_BR]].p;

	//경계구의 반지름
	m_fRadius = D3DXVec3Length(&v) / 2.f;
	m_vCenter = (pHeightMap[m_nCorner[DIR_TL]].p + pHeightMap[m_nCorner[DIR_BR]].p) / 2.f;

	if(_SubDivide() == false)
		return;

	for(int i=0; i<4; i++)
		m_pChild[i]->Build(pHeightMap);
}

void AVQuadTree::GetCorner( int& _0, int& _1, int& _2, int& _3 )
{
	_0 = m_nCorner[0];
	_1 = m_nCorner[1];
	_2 = m_nCorner[2];
	_3 = m_nCorner[3];
}

float AVQuadTree::_GetDistance( D3DXVECTOR3* pv1, D3DXVECTOR3* pv2 )
{
	return D3DXVec3Length( &(*pv2 - *pv1) );
}

int	AVQuadTree::_SearchVisibleTile(int tileNum, int nTiles, std::vector<int> *pV)
{
	if( m_bCulled )
	{
		m_bCulled = false;
		return tileNum;
	}

	//간격이 1이하면 leaf타일이다. 그러니까 더 안쪼개짐
	//그니까 너 임마 출력
	if( IsMinimum() )
	{
		int index = m_nCorner[DIR_TL] - (m_nCorner[DIR_TL] / (nTiles +1) );

		if(pV)	pV->push_back(index);
		tileNum++;

		return tileNum;
	}

	for(int i=0; i<4; i++)
		if( m_pChild[i] )
			tileNum = m_pChild[i]->_SearchVisibleTile( tileNum, nTiles, pV );

	return tileNum;
}

void AVQuadTree::Update(std::vector<int> *pV, TERRAIN_VERTEX *pHeightMap, AVFrustum *pFrustum, int &nTile, int &nCell)
{
	_FrustumCull(pHeightMap, pFrustum, nTile, nCell);
	pV->clear();

	_SearchVisibleTile(0, nTile, pV);
}

void AVQuadTree::Update(TERRAIN_VERTEX *pHeightMap, AVFrustum *pFrustum)
{
	_FrustumCull(pHeightMap, pFrustum);
}

PICK_DIR AVQuadTree::Picking(TERRAIN_VERTEX *pHeightMap, D3DXVECTOR2 &vMouse, int *pPickTL)
{
	AVPicking *picking = AVPicking::GetPicking(AVDirector::GetDiector()->GetApplication()->GetD3DDevice());

	if(m_bCulled)
		return PICK_DIR_NONE;

	if( picking->intersectSphere(AV_BOUNDING_SPHERE(m_vCenter, m_fRadius)) == false)
		return PICK_DIR_NONE;

	if( IsMinimum() )
	{
		float dist = 0.f;
		bool is = false;

		is = picking->intersectTri(
			&pHeightMap[m_nCorner[DIR_TL]].p, 
			&pHeightMap[m_nCorner[DIR_TR]].p, 
			&pHeightMap[m_nCorner[DIR_BL]].p, dist);
		picking->raySort(dist);

		if(is) 
		{
			if(pPickTL)
				*pPickTL = m_nCorner[DIR_TL];
			
			return PICK_DIR_UP;
		}

		is = picking->intersectTri(
			&pHeightMap[m_nCorner[DIR_BL]].p, 
			&pHeightMap[m_nCorner[DIR_TR]].p, 
			&pHeightMap[m_nCorner[DIR_BR]].p, dist);
		picking->raySort(dist);

		if(is)
		{
			if(pPickTL)
				*pPickTL = m_nCorner[DIR_TL];

			return PICK_DIR_DOWN;
		}
	}

	for(int i = 0; i<4; i++)
		if( m_pChild[i] )
			m_pChild[i]->Picking(pHeightMap, vMouse);
}