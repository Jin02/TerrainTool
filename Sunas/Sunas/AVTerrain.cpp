#include "AVTerrain.h"
#include "Dib.h"
#include "AVCamera.h"
#include "AVPicking.h"
#include "Information.h"

AVTerrain::AVTerrain(LPDIRECT3DDEVICE9 pDev, AVFrustum *pFrustum, AVCamera *pCamera)
{
	m_nTile				= 0;
	m_nTotalTiles		= 0;
	m_nSideVertex		= 0;

	m_nCell				= 0;
	m_nTileVertex		= 0;
	m_nTileIndices		= 0;
	m_nMaxLODLevel		= 0;
	m_nLODLevelSize		= 250;

	m_fScale			= 1.f;
	m_pd3dDevice		= NULL;
	m_ppIB				= NULL;

	m_pHeightMap		= NULL;
	m_pTileInfo			= NULL;

	m_pSLODTree			= NULL;
	m_pTerrainTree		= NULL;
	m_pHeightMap		= NULL;

	m_pFrustum			= NULL;
	m_pCamera			= NULL;
	m_isPick			= PICK_DIR_NONE;
	m_splatOption		= SPLATTING_OPTION_NONE;
	m_SplattingIndex	= 0;
	m_nBaseTexID		= 1;

	m_bLight = false;
	m_bFog = false;
	m_bWire = false;

	m_nBrushSize = 10;
	m_brushType = BRUSH_TYPE_NONE;


	m_pFrustum = pFrustum;
	m_pCamera = pCamera;
	m_pd3dDevice = pDev;
}

AVTerrain::~AVTerrain(void)
{
	_Destroy();
}

void AVTerrain::_SetMapInformation(int nTile, int nCell)
{
	m_nTile = nTile;
	m_nTotalTiles = nTile * nTile;
	m_nSideVertex = nTile * nCell + 1;

	m_nCell = nCell;
	m_nTileVertex = (nCell+1) * (nCell+1);
	m_nTileTriangles = nCell * nCell * 2;
	m_nTileIndices = m_nTileTriangles * 3;
}

bool AVTerrain::_CreateVertexBuffer(int index, TILE_INFO *pTileInfo)
{
	if( pTileInfo->pVB == NULL )
	{
		pTileInfo->pVertex = new TERRAIN_VERTEX[ m_nTileVertex ];

		if( FAILED( m_pd3dDevice->CreateVertexBuffer( m_nTileVertex * sizeof(TERRAIN_VERTEX),
			0, TERRAIN_VERTEX::FVF, D3DPOOL_DEFAULT, &pTileInfo->pVB, NULL ) ) )
		{
			return S_OK;
		}
	}

	//Ÿ�Ϻ��� ����ϴ� �Լ���

	int nextTileX = (index % m_nTile) * m_nCell;
	int nextTileZ = (index / m_nTile) * (m_nSideVertex * m_nCell);

	//�����ؽ��� 1���� ��� ������ ����Ǵϱ�, �̷��� �ϴ°ž�
	float alphaTu = (float)(index % m_nTile) / (float)m_nTile;
	float alphaTv = (float)(index / m_nTile) / (float)m_nTile;

	int i=0;
	int z=0;
	int x=0;
	for( z=0; z<m_nCell+1; ++z )
		for( x=0; x<m_nCell+1; ++x )
		{
			pTileInfo->pVertex[i]		= m_pHeightMap[ nextTileZ + nextTileX + (z * m_nSideVertex) + x ];
			pTileInfo->pVertex[i].t.x	= (float)x / (float)(m_nCell); // Ÿ�Ϻ��� 0.0 ~ 1.0���� ��´�.
			pTileInfo->pVertex[i].t.y	= (float)z / (float)(m_nCell); // �ؽ�ó ��Ʈ������ �̿��ؼ� ��������.
			pTileInfo->pVertex[i].t2.x	= alphaTu + ((float)x / (float)(m_nSideVertex-1));
			pTileInfo->pVertex[i].t2.y	= alphaTv + ((float)z / (float)(m_nSideVertex-1));
			i++;
		}

		pTileInfo->nCorner[0]		= nextTileZ + nextTileX;
		pTileInfo->nCorner[1]		= nextTileZ + nextTileX + m_nCell;
		pTileInfo->nCorner[2]		= nextTileZ + nextTileX + (m_nCell * m_nSideVertex);
		pTileInfo->nCorner[3]		= nextTileZ + nextTileX + (m_nCell * m_nSideVertex) + m_nCell;

		pTileInfo->nCenter			= (pTileInfo->nCorner[0] + pTileInfo->nCorner[1] + pTileInfo->nCorner[2] + pTileInfo->nCorner[3]) / 4;

		pTileInfo->nLodLevel			= 0;
		pTileInfo->nLodCrackIndex	= 0;
		pTileInfo->nTriangleNum		= m_nCell * m_nCell * 2;

		TERRAIN_VERTEX* pV;

		if( FAILED( pTileInfo->pVB->Lock( 0, m_nTileVertex * sizeof(TERRAIN_VERTEX), (void**)&pV, NULL ) ) )
			return false;

		memcpy( pV, pTileInfo->pVertex, m_nTileVertex * sizeof(TERRAIN_VERTEX) );
		pTileInfo->pVB->Unlock();

		return true;
}

bool AVTerrain::_CreateIndexBuffer(int LODLevel)
{
	int level	= (int)pow( 4.f, LODLevel );		// �������� (1, 4, 16���� 4�� �������� Ŀ���� �Ѵ�
	int size	= m_nTileIndices / level;	// �ε��� ������(�������� 4�� ���������� ������ �Ѵ�)
	int next	= (int)pow( 2.f, LODLevel );		// ���� ���ؽ��� �Ѿ�� ũ��

	// LOD �������� ũ�� �ε��� ������ ����ϴ�.
	int crackIndex=0;
	for( crackIndex=0; crackIndex<5; crackIndex++ )
	{
		// 0��° ������ ��� ũ�� �ε��� ���۰� �ʿ�����Ƿ� �н�
		if( LODLevel == 0 && crackIndex > 0 )
		{
			m_ppIB[LODLevel][crackIndex] = NULL;
			continue;
		}

		// 0�� �⺻ ũ�� �ε��� ����
		int indexSize = size;
		if( 0 < LODLevel && (1 <= crackIndex && crackIndex < 5) )	// 1 ~ 4�� ���� cell�� 3���� �ﰢ�� �߰�
		{
			// ���� �����ڸ� ������ 3���� �ﰢ�� �߰�(1�ﰢ���� 3�ε���)
			indexSize += (m_nCell / next) * 3 * 3;
		}


		D3DFORMAT foramt;

#ifdef USE_INDEX_16
		foramt = D3DFMT_INDEX16;
#else
		foramt = D3DFMT_INDEX32;
#endif

		if( FAILED( m_pd3dDevice->CreateIndexBuffer( indexSize * (sizeof(TRIANGLE_INDEX)/3),
			0, foramt, D3DPOOL_DEFAULT, &m_ppIB[LODLevel][crackIndex], NULL ) ) )
		{ return false; }

		WORD* pI;
		if( FAILED( m_ppIB[LODLevel][crackIndex]->Lock( 0, indexSize * (sizeof(TRIANGLE_INDEX)/3), 
			(void**)&pI, NULL ) ) )
		{ return false; }

		int z=0;
		int x=0;
		int half = next/2;

		for( z=0; z<m_nCell; z+=next )
			for( x=0; x<m_nCell; x+=next )
			{
				// 0--1--2
				// 3--4--5
				// 6--7--8
				int _0 = 	    z * (m_nCell+1) + x;		// 0
				int _1 = 	    z * (m_nCell+1) + x+half;	// 1
				int _2 = 	    z * (m_nCell+1) + x+next;	// 2
				int _3 = (z+half) * (m_nCell+1) + x;		// 3
				int _4 = (z+half) * (m_nCell+1) + x+half;	// 4
				int _5 = (z+half) * (m_nCell+1) + x+next;	// 5
				int _6 = (z+next) * (m_nCell+1) + x;		// 6
				int _7 = (z+next) * (m_nCell+1) + x+half;	// 7
				int _8 = (z+next) * (m_nCell+1) + x+next;	// 8

				bool bCheck = false;
				switch( crackIndex )
				{
				case CRACK_LOD_NORMAL:
					bCheck = true;
					break;

					// LEFT
				case CRACK_LOD_LEFT:
					if( x == 0 )
					{
						*pI++ =	_4;		*pI++ = _0;		*pI++ = _2;	// 4, 0, 2
						*pI++ = _4;		*pI++ = _2;		*pI++ = _8;	// 4, 2, 8
						*pI++ = _4;		*pI++ = _8;		*pI++ = _6;	// 4, 8, 6
						*pI++ = _4;		*pI++ = _6;		*pI++ = _3;	// 4, 6, 3
						*pI++ = _4;		*pI++ = _3;		*pI++ = _0;	// 4, 3, 0
						bCheck = false;
					}
					else{ bCheck = true; }
					break;

					// TOP
				case CRACK_LOD_TOP:
					if( z == 0 )
					{
						*pI++ =	_4;		*pI++ = _0;		*pI++ = _1;	// 4, 0, 1
						*pI++ =	_4;		*pI++ = _1;		*pI++ = _2;	// 4, 1, 2
						*pI++ =	_4;		*pI++ = _2;		*pI++ = _8;	// 4, 2, 8
						*pI++ =	_4;		*pI++ = _8;		*pI++ = _6;	// 4, 8, 6
						*pI++ =	_4;		*pI++ = _6;		*pI++ = _0;	// 4, 6, 0
						bCheck = false;
					}
					else{ bCheck = true; }
					break;

					// RIGHT
				case CRACK_LOD_RIGHT:
					if( x == m_nCell-next )
					{
						*pI++ =	_4;		*pI++ = _0;		*pI++ = _2;	// 4, 0, 2
						*pI++ =	_4;		*pI++ = _2;		*pI++ = _5;	// 4, 2, 5
						*pI++ =	_4;		*pI++ = _5;		*pI++ = _8;	// 4, 5, 8
						*pI++ =	_4;		*pI++ = _8;		*pI++ = _6;	// 4, 8, 6
						*pI++ =	_4;		*pI++ = _6;		*pI++ = _0;	// 4, 6, 0
						bCheck = false;
					}
					else{ bCheck = true; }
					break;			

					// BOTTOM
				case CRACK_LOD_BOTTOM:
					if( z == m_nCell-next )
					{
						*pI++ =	_4;		*pI++ = _0;		*pI++ = _2;	// 4, 0, 2
						*pI++ =	_4;		*pI++ = _2;		*pI++ = _8;	// 4, 2, 8
						*pI++ =	_4;		*pI++ = _8;		*pI++ = _7;	// 4, 8, 7
						*pI++ =	_4;		*pI++ = _7;		*pI++ = _6;	// 4, 7, 6
						*pI++ =	_4;		*pI++ = _6;		*pI++ = _0;	// 4, 6, 0
						bCheck = false;
					}
					else{ bCheck = true; }
					break;

				} // end case

				if( bCheck )
				{
					*pI++ =	_0;		*pI++ = _2;		*pI++ = _6;	// 0, 2, 6
					*pI++ =	_6;		*pI++ = _2;		*pI++ = _8;	// 6, 2, 8
				}
			} // end for

			m_ppIB[LODLevel][crackIndex]->Unlock();
	}

	return true;
}

bool AVTerrain::Create(float fScale, wchar_t *baseImg, int nCell, int nTile, std::vector<D3DXVECTOR3>* pvHeightMap, vector<pair<int, BYTE*>>* pvSplatting, vector<int>*	pvSplattingVisibleTiles)
{	
	m_fScale = fScale;
	_SetMapInformation(nTile, nCell);

	if(_BuildHeightMap() == false)
	{
		_Destroy();
		return false;
	}

	//�����غ��ϱ�; ���� Ÿ���� ������ �ʿ��Ұ� ����
	//��ü Ÿ�������� �ʿ��Ұ� ���⵵��

	m_pTileInfo = new TILE_INFO[m_nTotalTiles];

	for(int i=0; i<m_nTotalTiles; i++)
	{
		if( _CreateVertexBuffer(i, &m_pTileInfo[i]) == false )
		{
			_Destroy();
			return false;
		}
	}

	int devideCell = m_nCell;

	while( devideCell > 2 )
	{
		devideCell /= 2;
		m_nMaxLODLevel++;
	}

	m_ppIB = new LPDIRECT3DINDEXBUFFER9[m_nMaxLODLevel][5];

	for(int i=0; i<m_nMaxLODLevel; i++)
	{
		if( _CreateIndexBuffer(i) == false )
		{
			_Destroy();
			return false;
		}
	}

	TEXTURE *tex = SharedTexture::GetSharedTexture()->GetTexture(baseImg);
	m_nBaseTexID = tex->id;

	m_pSLODTree = new AVQuadTree(m_nTile);
	m_pSLODTree->Build(m_pHeightMap, m_nCell, m_nTile);

	m_pTerrainTree = new AVQuadTree(m_nSideVertex - 1);
	m_pTerrainTree->Build(m_pHeightMap);

	if( pvHeightMap != NULL )
	{
		int size = m_nSideVertex * m_nSideVertex -1;

		std::vector<D3DXVECTOR3>::iterator iter;
		int i = 0;
		for(iter = pvHeightMap->begin(); iter != pvHeightMap->end(); iter++)
			m_pHeightMap[i++].p = (*iter);

		for(i=0; i<m_nTotalTiles; ++i)
			_CreateVertexBuffer(i, &m_pTileInfo[i]);
		
		int x[3], z[3];
		x[0] = z[0] = 0;
		x[1] = m_nSideVertex;
		z[1] = 0;
		x[2] = 0;
		z[2] = m_nSideVertex;

		_CalcNormalVector(x,z);
	}

	if( pvSplatting != NULL )
	{
		std::vector<pair<int, BYTE*>>::iterator iter;
		int id = 0;
		for(iter = pvSplatting->begin(); iter != pvSplatting->end(); ++iter)
		{
			int index= (*iter).first;
			BYTE *bits = (*iter).second;
			AddSplattingTexture(index, id);
			m_vSplatting[id++]->load(bits);
		}

		int size = pvSplattingVisibleTiles->size();

		for(int i=0; i<size; ++i)
			m_vSplattingTiles.push_back((*pvSplattingVisibleTiles)[i]);
	}


	return true;
}

void AVTerrain::_Destroy()
{
	SAFE_DELETE(m_pSLODTree);
	SAFE_DELETE(m_pTerrainTree);
	//Ÿ�ϸ��� vb����

	for( int i=0; i<m_nMaxLODLevel; ++i )
		for( int j=0; j<5; ++j )
			SAFE_RELEASE( m_ppIB[i][j] );

	SAFE_ARRARY_DELETE(m_pHeightMap);
	SAFE_ARRARY_DELETE(m_pTileInfo);
}

bool AVTerrain::_BuildHeightMap( )
{
	int		n;

	n = Log2( m_nSideVertex );
	int a = ((int)pow( 2.f, 7 ) + 1);
	if( ((int)pow( 2.f, n ) + 1) != m_nSideVertex ) return false;
	n = Log2( m_nSideVertex );
	if( ((int)pow( 2.f, n ) + 1) != m_nSideVertex ) return false;

	m_pHeightMap = new TERRAIN_VERTEX[m_nSideVertex * m_nSideVertex];
	TERRAIN_VERTEX v;

	for( int z = 0 ; z < m_nSideVertex ; z++ )
	{
		for( int x = 0 ; x < m_nSideVertex ; x++ )
		{
			//ȭ�� �� ����� 0,0�� ���ߴ°͵� ������ ���߿� ����Ҷ� ��������
			//	v.p.x = (float)( ( x - m_nTile / 2 ) * m_vfScale.x );
			//	v.p.z = -(float)( ( z - m_nTile / 2 ) * m_vfScale.z );
			v.p.x = (float)x * m_fScale;
			v.p.z = -(float)z * m_fScale;
			v.p.y = 0.f;

			v.n.x = 0.f;
			v.n.y = 1.f;
			v.n.z = 0.f;

			D3DXVec3Normalize(&v.n, &v.n);

			v.t.x = 0.f;
			v.t.y = 0.f;
			v.t2.x = 0.f;
			v.t2.y = 0.f;

			m_pHeightMap[x + z * m_nSideVertex] = v;
		}
	}

	return true;
}

LPDIRECT3DTEXTURE9 AVTerrain::AddTexture( TEXTURE *tex )
{
	m_vTex.push_back( tex->texture );
	return tex->texture;
}

void AVTerrain::SetLOD(D3DXVECTOR3 &vLook, int index)
{
	float LengthX = abs(vLook.x - m_pHeightMap[m_pTileInfo[index].nCenter].p.x);
	float LengthZ = abs(vLook.z - m_pHeightMap[m_pTileInfo[index].nCenter].p.z);

	float Len = LengthX >= LengthZ ? LengthX : LengthZ;

	int i=0;
	for( i=0; i<m_nMaxLODLevel; ++i )
	{
		if( (float)(i*m_nLODLevelSize) <= Len )
			m_pTileInfo[index].nLodLevel = i;
		else
			break;
	}
}

void AVTerrain::SetCrack(int index)
{
	//	enum { LEFT, TOP, RIGHT, BOTTOM };

	int current	= m_pTileInfo[index].nLodLevel;
	int cmp[4] = { 0, };
	int tiles	= m_nTile;

	cmp[0]	= ( index%tiles > 0 )		? m_pTileInfo[index-1].nLodLevel			: m_pTileInfo[index].nLodLevel;
	cmp[1]	= ( index/tiles > 0 )		? m_pTileInfo[index-tiles].nLodLevel		: m_pTileInfo[index].nLodLevel;
	cmp[2]	= ( index%tiles < tiles-1 ) ? m_pTileInfo[index+1].nLodLevel			: m_pTileInfo[index].nLodLevel;
	cmp[3]	= ( index/tiles < tiles-1 ) ? m_pTileInfo[index+tiles].nLodLevel		: m_pTileInfo[index].nLodLevel;

	int devide = (int)pow( 2.f, m_pTileInfo[index].nLodLevel );
	int triNum = m_nCell / devide;

	int i=0;
	int count=0;
	for( i=0; i<4; ++i )
	{
		// 3 2 6
		// 1 0 4
		// 9 8 12
		// ���� ����ó�� left�� ���ؼ� ������ Ʋ����� count=1
		// left, up 2���� 1+2=3�� �ȴ�. up, right 2���� 2+4=6 ����� �ߺ����� �ʴ´�.
		// �� 2���� Ʋ�� ���� �� ��ġ�� �ش��ϴ� �밢�� ���ڰ� ������ �ȴ�.

		// left, up, right, down�� ���ϸ鼭 ���緹���� �ֺ��������� Ŭ ��츸 �����Ѵ�.
		// ������ ������ Ŭ ���� ���η����� ������ �ϱ� �����̴�.
		if( current <= cmp[i] )		// ���緹���� �ֺ� �������� ���� ���
			count += 0;
		else if( current > cmp[i] )	// ���緹���� �ֺ��������� Ŭ ���
			count += (int)pow( 2.f, i );
	}

	// 0, 3, 6, 9, 12�� ���� �����̿��� 
	if( count % 3 == 0 )
		count = 0;

	switch( count )
	{
	case 0:
		m_pTileInfo[index].nLodCrackIndex	= CRACK_LOD_NORMAL;
		m_pTileInfo[index].nTriangleNum		= triNum*triNum*2;
		break;

	case 1:
		m_pTileInfo[index].nLodCrackIndex	= CRACK_LOD_LEFT;
		m_pTileInfo[index].nTriangleNum		= triNum*triNum*2 + (triNum*3);
		break;

	case 2:
		m_pTileInfo[index].nLodCrackIndex	= CRACK_LOD_TOP;
		m_pTileInfo[index].nTriangleNum		= triNum*triNum*2 + (triNum*3);
		break;

	case 4:
		m_pTileInfo[index].nLodCrackIndex	= CRACK_LOD_RIGHT;
		m_pTileInfo[index].nTriangleNum		= triNum*triNum*2 + (triNum*3);
		break;

	case 8:
		m_pTileInfo[index].nLodCrackIndex	= CRACK_LOD_BOTTOM;
		m_pTileInfo[index].nTriangleNum		= triNum*triNum*2 + (triNum*3);
		break;
	}
}

void AVTerrain::_textureSetting(int index, LPDIRECT3DTEXTURE9 tex)
{
	m_pd3dDevice->SetTexture( index, tex );
	m_pd3dDevice->SetSamplerState( index, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_pd3dDevice->SetTextureStageState( index, D3DTSS_TEXCOORDINDEX, 0 );
}

bool AVTerrain::Update()
{
	m_pSLODTree->Update(&m_vVisibleTiles, m_pHeightMap, m_pFrustum, m_nTile, m_nCell);
	m_pTerrainTree->Update(m_pHeightMap, m_pFrustum);

	for(int i=0; i<m_nTotalTiles; i++)
		SetLOD(m_pCamera->getPosition(),i);


	//m_vVisibleTiles�� QuadTree���� �ø��Ǿ�
	//����Ҽ��ִ� ������ ��Ÿ����.
	//�̰ɷ� slod�� ũ���� �����ϰ�, ��Ÿ�����ִ� Ÿ���� ��ŭ
	//���⼭ ��ŷ���� �ؾ��Ѵ�
	//�ϴ� ���� �Ͼ�� Ÿ�Ϲ�ȣ�� ���� ���� ������ �ε��� ��ȣ���� ���
	//������ ������ �غ��� �ִ°ű��ص� �����ϸ� �� �����ϱ� �غ���
	//��ư �װ� ������ �� Ÿ���̶� ���� �����߰��� ��ư ã��


	std::vector<int>::iterator iter;
	for(iter = m_vVisibleTiles.begin(); iter != m_vVisibleTiles.end(); iter++)
		SetCrack((*iter));

	if(m_splatOption != SPLATTING_OPTION_NONE )
	{
		D3DXVECTOR3 p = AVPicking::GetPicking(m_pd3dDevice)->GetPickPos();
		bool isAdd = true;

		if(m_splatOption == SPLATTING_OPTION_MINUS)
			isAdd = false;

		m_vSplatting[m_SplattingIndex]->AlphaMapMode(isAdd);
		m_vSplatting[m_SplattingIndex]->AlphaMapDraw(m_nTile * m_nCell * m_fScale, m_nBrushSize * 2 * m_fScale, p);

		if(m_splatOption == SPLATTING_OPTION_ADD)
			_SearchSplattingTiles(p, m_nBrushSize * 2 * m_fScale);
		else
		{
			//���ִ°ſ����� �� ���ֱ� �ؾߴµ�,
			//�ϴ� ������ �ٸ��� ���Ѱ� ����
		}
	}

	return true;
}

void AVTerrain::_BaseRender(int index)
{
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );	// 0�� �ؽ�ó ���������� Ȯ�� ����
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );	// 0�� �ؽ�ó ���������� ��� ����

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );		// 0�� �ؽ�ó : 0�� �ؽ�ó �ε��� ���
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );		// MODULATE�� ���´�.
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );		// �ؽ�ó
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );		// ������
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

	m_pd3dDevice->SetTexture( 0, m_vTex[m_nBaseTexID] );
	m_pd3dDevice->SetStreamSource( 0, m_pTileInfo[index].pVB, 0, sizeof(TERRAIN_VERTEX) );
	m_pd3dDevice->SetFVF( TERRAIN_VERTEX::FVF );
	m_pd3dDevice->SetIndices( m_ppIB[m_pTileInfo[index].nLodLevel][m_pTileInfo[index].nLodCrackIndex] );
	m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_nTileVertex, 0, m_pTileInfo[index].nTriangleNum );
}

void AVTerrain::_SplattingRender(int index)
{
	for(int i=0; i<m_vSplatting.size(); ++i )
	{
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 1 );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

		m_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
		m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,	D3DTOP_MODULATE );
		m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
		m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
		m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);

		// �⺻ ���� �ɼ�
		m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

		m_pd3dDevice->SetTexture( 0, m_vSplatting[i]->AlphaTexture() );
		m_pd3dDevice->SetTexture( 1, m_vSplatting[i]->SplatTexture() );
		m_pd3dDevice->SetStreamSource( 0, m_pTileInfo[index].pVB, 0, sizeof(TERRAIN_VERTEX) );
		m_pd3dDevice->SetIndices( m_ppIB[m_pTileInfo[index].nLodLevel][m_pTileInfo[index].nLodCrackIndex] );
		m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_nTileVertex, 0, m_pTileInfo[index].nTriangleNum );
	}

	// ��Ƽ�ؽ��� ������ ����� ���� �Ѵ�.
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DISABLE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	// ���� ����� ���� �Ѵ�.
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );	
}

bool AVTerrain::Render()
{
	D3DXVECTOR3 *vPick = &AVPicking::GetPicking(m_pd3dDevice)->GetPickPos();

	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, m_bLight );
	m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE, m_bFog );
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE,  m_bWire ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	for(int i=0; i<m_vVisibleTiles.size(); ++i )
		_BaseRender( m_vVisibleTiles[i] );

	for(int i=0; i<m_vSplattingTiles.size(); ++i )
	{
		bool bVisible=false;
		for(int j=0; j<m_vVisibleTiles.size(); ++j )
		{
			if( m_vVisibleTiles[j] == m_vSplattingTiles[i] )
				bVisible = true;
		}
		if( bVisible )
			_SplattingRender( m_vSplattingTiles[i] );
	}

	m_pd3dDevice->SetTexture(0,NULL);

	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

	if(m_brushType == BRUSH_TYPE_CIRCLE)
		DrawCircleBrush(*vPick, static_cast<float>(m_nBrushSize));
	if(m_brushType == BRUSH_TYPE_SQUARE)
		DrawSquareBrush(*vPick, static_cast<float>(m_nBrushSize));

	DrawDotBrush(*vPick);

	return true;
}

std::vector<int>* AVTerrain::GetVisibleTiles()
{
	return &m_vVisibleTiles;
}

int AVTerrain::GetTilesNum()
{
	return m_nTile;
}

int AVTerrain::GetCellsNum()
{
	return m_nCell;
}

TERRAIN_VERTEX* AVTerrain::GetHeightMap()
{
	return m_pHeightMap;
}

bool AVTerrain::Picking(D3DXVECTOR2 &vMouse)
{
	int tl;
	m_isPick	= m_pTerrainTree->Picking(m_pHeightMap, vMouse, &tl);

	return (bool)m_isPick;
}

float AVTerrain::GetHeight(float x, float z)
{
	x =  (x / m_fScale); //������, Ÿ�������� �ٲ��ش�
	z = -(z / m_fScale);

	//�ϴ�, ���콺�� �� �ȿ� �ִ��� Ȯ���ؾ���
	//��ŷ�� �� ó�� ���ϱ� �ߴµ�, ���⼭ ó�� ���Ѵٸ� ���������̰���

	if( x <= 0.f || z <= 0.f ||	x >= (m_nSideVertex - 1) ||	z >= (m_nSideVertex - 1) )
		return 0.f; //���콺 ������ ���� �������̴�, 0.f�� �����Ѵ�.

	int nx = (int)x;
	int nz = (int)z;

	float tileQuad[4];

	// tl 0
	// tr 1
	// bl 2
	// br 3

	tileQuad[0] = m_pHeightMap[ nz     * m_nSideVertex + nx    ].p.y;
	tileQuad[1] = m_pHeightMap[ nz     * m_nSideVertex + nx +1 ].p.y;
	tileQuad[2] = m_pHeightMap[ (nz+1) * m_nSideVertex + nx    ].p.y;
	tileQuad[3] = m_pHeightMap[ (nz+1) * m_nSideVertex + nx +1 ].p.y;

	float dx = x - nx; //�� �Ҽ��� ���
	float dz = z - nz; //���� ���� 5�þ�
	float height = 0.f;

	//������ �����Ѵٸ� nx, nz�� ū �������ݾ�.
	//�� �װɷ� ���� ������ ã�µ� ��ٰ� �����ϸ��
	//nx nz�� �뷫���� ��, dx dz�� �������� ���̶�� �����ϸ� �Ƿ���
	//�ϴ� nx nz�� �뷫���� Ÿ�� �����, dx dz�� �������̴ϱ�
	//�̰ɷ� ���̸� ã�����.
	//���������� ����ϸ� �ɰž� ��¥�� �������̴ϱ� õõ�� �����غ�

	//���� �簢���� ������ �����ϸ� �̰� ���� ����̱��� ��� ������
	//��ž�
	if(dz < 1.f - dx)  // ���� �ﰢ��
	{
		float uy = tileQuad[1] - tileQuad[0]; 
		float vy = tileQuad[2] - tileQuad[0];

		height = tileQuad[0] + Lerp(0.0f, uy, dx) + Lerp(0.0f, vy, dz);
	}
	else // �Ʒ��� �ﰢ�� DCB
	{
		float uy = tileQuad[2] - tileQuad[3];
		float vy = tileQuad[1] - tileQuad[3];

		height = tileQuad[3] + Lerp(0.0f, uy, 1.0f - dx) + Lerp(0.0f, vy, 1.0f - dz);
	}

	return height;
}

void AVTerrain::DrawCircleBrush(D3DXVECTOR3 &vPickPos, float size, int count)
{
	//static LPDIRECT3DDEVICE9 pDevice = AVDirector::GetDiector()->GetApplication()->GetD3DDevice();
	float radian		= D3DX_PI * 2 / count;

	BRUSH_VERTEX	line[2];
	D3DXVECTOR3		curPos(1.f, 0.f, 0.f);
	D3DXVECTOR3		newPos;//�̾��� ��ġ

	D3DXMATRIX		mat;
	D3DXMatrixIdentity(&mat);

	line[1].p		= curPos * size + vPickPos;
	line[1].p.y		= GetHeight( line[1].p.x, line[1].p.z ) + 0.5f;
	line[1].color	= line[0].color = 0xffff0000;//D3DCOLOR_ARGB(255,255,0,0);


	for(int i=1; i<=count; i++)
	{
		line[0].p = line[1].p;

		D3DXMatrixRotationY( &mat, i*radian );
		D3DXVec3TransformCoord( &newPos, &curPos, &mat );
		D3DXVec3Normalize( &newPos, &newPos );

		line[1].p	= newPos * size + vPickPos;
		line[1].p.y	= GetHeight( line[1].p.x, line[1].p.z ) + 0.5f;

		m_pd3dDevice->SetFVF( BRUSH_VERTEX::FVF );
		m_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, 1, line, sizeof(BRUSH_VERTEX) );
	}
}

void AVTerrain::DrawSquareBrush(D3DXVECTOR3 &vPickPos, float size, int count)
{
	//static LPDIRECT3DDEVICE9 pDevice = AVDirector::GetDiector()->GetApplication()->GetD3DDevice();
	count /= 4; //�簢���̴ϱ�,

	BRUSH_VERTEX	line[2];
	D3DXVECTOR3		curPos(1.f, 0.f, 0.f);
	D3DXVECTOR3		newPos;//�̾��� ��ġ

	D3DXMATRIX		mat;
	D3DXMatrixIdentity(&mat);

	line[1].p		= curPos * size + vPickPos;
	line[1].p.y		= GetHeight( line[1].p.x, line[1].p.z ) + 0.5f;
	line[1].color	= line[0].color = 0xffff0000;//D3DCOLOR_ARGB(255,255,0,0);


	D3DXVECTOR3 vTL( -1.0f, 0.0f, +1.0f );
	D3DXVECTOR3 vTR( +1.0f, 0.0f, +1.0f );
	D3DXVECTOR3 vBL( -1.0f, 0.0f, -1.0f );
	D3DXVECTOR3 vBR( +1.0f, 0.0f, -1.0f );
	//1�� ���� ������ ���� ���� �׳� ��Į�� ���� �������µ�
	//������ ���� �����ݾ� �� ���� ���� �������פ� ���� ������ 9��

	float t = 0.f;
	D3DXVECTOR3 p1(0,0,0);
	D3DXVECTOR3 p2(0,0,0);

	for(int quad = 0; quad < 4; quad++)
	{
		if( quad == 0 )
			p1 = vTL * size,	p2 = vTR * size;
		else if( quad == 1 )
			p1 = vTL * size,	p2 = vBL * size;
		else if( quad == 2 )
			p1 = vBL * size,	p2 = vBR * size;
		else // quad == 4
			p1 = vTR * size,	p2 = vBR * size;

		p1 += vPickPos;
		p2 += vPickPos;

		// p(t) = (1-t)p1 +(t)p2 �� cgå 4�ܿ��� �մ���..
		for(int i=0; i<count; i++)
		{
			t = ((float)(i) / (int)count);
			line[0].p		= (1.0f - t)*p1 + (t)*p2;
			line[0].p.y		= GetHeight( line[0].p.x, line[0].p.z ) + 0.5f;

			t = ((float)(i+1) / (int)count);
			line[1].p		= (1.0f - t)*p1 + (t)*p2;
			line[1].p.y		= GetHeight( line[1].p.x, line[1].p.z ) + 0.5f;

			m_pd3dDevice->SetFVF( BRUSH_VERTEX::FVF );
			m_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, 1, line, sizeof(BRUSH_VERTEX) );
		}
	}
}

void AVTerrain::DrawDotBrush(D3DXVECTOR3 &vPickPos)
{
	BRUSH_VERTEX dot;
	dot.color = 0xffff0000;
	dot.p	  = vPickPos;
	dot.p.y	  = GetHeight( vPickPos.x, vPickPos.z ) + 0.5f;
	m_pd3dDevice->SetFVF( BRUSH_VERTEX::FVF );
	m_pd3dDevice->DrawPrimitiveUP( D3DPT_POINTLIST, 1, &dot, sizeof(BRUSH_VERTEX) );
}

void AVTerrain::EditTerrain(D3DXVECTOR3 &vPickPos, BRUSH_TYPE brushType, float size, float editRate, EDIT_TERRAIN_OPTION option)
{
	float fx, fz;

	fx =	  vPickPos.x / m_fScale;
	fz = -(vPickPos.z / m_fScale);

	if( fx <= 0.f || fz <= 0.f ||	fx >= (m_nSideVertex - 1) ||	 fz >= (m_nSideVertex - 1) )
		return; //���콺 ������ ���� �������̴�, 0.f�� �����Ѵ�.

	int startX	= fx - size / m_fScale;
	int startZ	= fz - size / m_fScale;
	int endX	= fx + size / m_fScale;
	int endZ	= fz + size / m_fScale;

	if(startX < 0)			 startX	= 0;
	if(startZ < 0)			 startZ	= 0;
	if(endX	> m_nSideVertex) endX	= m_nSideVertex;
	if(endZ > m_nSideVertex) endZ	= m_nSideVertex;

	if(brushType == BRUSH_TYPE_SQUARE)
	{
		int leftTop  = startZ*m_nSideVertex+startX-1;
		int rightBot = endZ*m_nSideVertex+endX+1;

		if(leftTop  < 0 ) leftTop = 0;
		if(rightBot > m_nSideVertex * m_nSideVertex )
			rightBot = m_nSideVertex * m_nSideVertex;

		D3DXVECTOR3 p1 = m_pHeightMap[leftTop].p;
		D3DXVECTOR3 p2 = m_pHeightMap[rightBot].p;
		size = Distance(p1, p2);
	}

	for(int z = startZ; z <= endZ; z++)
	{
		for(int x = startX; x <= endX; x++)
		{
			D3DXVECTOR3 *p = &m_pHeightMap[z*m_nSideVertex+x].p;

			float	distance		= Distance(vPickPos, *p);

			if( brushType == BRUSH_TYPE_CIRCLE )
				if(distance >= size)
					continue;

			if( option == EDIT_TERRAIN_OPTION_FLAT )
				p->y += editRate;
			else if( option == EDIT_TERRAIN_OPTION_CRATER )
				p->y += distance / size * editRate;
			else
				p->y += (1.f - distance / size) * editRate;
		}
	}

	std::vector<int> vAreaTile = SearchTileArea(vPickPos, size);
	std::vector<int>::iterator iterator;

	for( iterator = vAreaTile.begin(); iterator != vAreaTile.end(); iterator++ )
		_CreateVertexBuffer(*iterator, &m_pTileInfo[*iterator]);
}

std::vector<int> AVTerrain::SearchTileArea(D3DXVECTOR3 &vPickPos, float size)
{
	std::vector<int> vTileIndex;
	float fx, fz;

	fx =   vPickPos.x / m_fScale;
	fz = -(vPickPos.z / m_fScale);

	int startX	= (fx - (size / m_fScale)) / m_nCell-1;
	int startZ	= (fz - (size / m_fScale)) / m_nCell-1;
	int endX	= (fx + (size / m_fScale)) / m_nCell+1;
	int endZ	= (fz + (size / m_fScale)) / m_nCell+1;

	if(startX < 0) startX = 0;
	if(startZ < 0) startZ = 0;
	if(endX > m_nTile-1) endX = m_nTile-1;
	if(endZ > m_nTile-1) endZ = m_nTile-1;

	for(int z = startZ; z <= endZ; z++)
	{
		for(int x = startX; x <= endX; x++)
		{
			int index = x + (z * m_nTile);
			vTileIndex.push_back(index);
		}
	}

	//��� ����ؾߵ�..
	//��.. �׷�.. ��.. ����
	int x[3], z[3];
	x[0] = z[0] = m_pTileInfo[ startX ].nCorner[0];
	x[1] = z[1] = m_pTileInfo[ endX	  ].nCorner[1];
	x[2] = z[2] = m_pTileInfo[ endZ	  ].nCorner[2];

	//sv�� ������, x�� z�� �װ� ��
	//�״ϱ� �̰� ���ĸ�, ��.. �����ؼ� �־���Ұ� ���� ���õ�
	//���� Ÿ�Ͽ��� tl, tr, bl�� �����ؾ����ݾ�
	//�׷� �׷��ϱ� �׷��� �ϸ��
	//�׷��ϱ� x���� 1�� �����ݾ�. �׷� �Ʒ����� �ɷ����°� �ٽ� 1�̵ǰ���
	//���� ���� �����ؾ��ұ�, 
	//�� �׷�. ���� �����̶�� �����Ҷ�, �� �Ʒ� ������ ����ϸ�
	//��������, ���°�ٲ�, �� �̷��� ���� ���� ���� ���� �̷���
	for(int i=0; i<3; i++)
	{
		x[i] = x[i] % m_nSideVertex;
		z[i] = z[i] / m_nSideVertex;
	}

	_CalcNormalVector(x,z);

	return vTileIndex;
}

bool AVTerrain::AddSplattingTexture(int index, int id)
{
	if(m_vSplatting.size() >= 5)
		return false;
	if( index+1 > TILE_TEXTURE_NUM )
		return false;

	AVSplatting *pSplatting = new AVSplatting(m_pd3dDevice);
	pSplatting->Create(256, m_vTex[index], id, index);
	m_vSplatting.push_back(pSplatting);

	return true;
}

void AVTerrain::DeleteSplattingTexture(int id)
{
	std::vector<AVSplatting*>::iterator iter;

	for(iter = m_vSplatting.begin(); iter != m_vSplatting.end(); ++iter)
	{
		if((*iter)->Id() == id)
		{
			m_vSplatting.erase(iter);
			return;
		}
	}
}

bool AVTerrain::SetSplattingTexture(int id)
{
	//std::vector<AVSplatting*>::iterator iter;
	//int index = 0;
	//for(iter = m_vSplatting.begin(); iter != m_vSplatting.end(); iter++)
	//{
	//	if((*iter)->Id() == id)
	//	{
	//		m_SplattingIndex = (*iter)->Id();
	//		return true;
	//	}

	//	index++;
	//}

	int index = 0;
	std::vector<AVSplatting*>::iterator iter;
	for(iter = m_vSplatting.begin(); iter != m_vSplatting.end(); iter++)
	{
		if((*iter)->Id() == id)
		{
			m_SplattingIndex = index;
			return true;
		}

		index++;
	}

	return false;
}

int AVTerrain::SearchSplattingTex(LPDIRECT3DTEXTURE9 tex)
{
	int i;

	for(i = 0; i < m_vSplatting.size(); i++)
	{
		if( m_vSplatting[i]->Texture() == tex )
			return i;
	}

	return -1;
}

int AVTerrain::_SearchPositionInTile(D3DXVECTOR3 &vPosition)
{
	// ��ǥ�� ����� Ÿ�Ϲ�ȣ�� ������ �ʴ´�.
	if( vPosition.x < 0 || vPosition.z > 0 )
		return -1;

	// ���� ��ġ�� Ÿ�� ��ȣ�� ���Ѵ�. 
	int x =  (int)(vPosition.x * m_nTile) / (m_nTile * m_nCell * m_fScale);
	int z = -(int)(vPosition.z * m_nTile) / (m_nTile * m_nCell * m_fScale);

	return  z * m_nTile + x;
}

void AVTerrain::_SearchSplattingTiles(D3DXVECTOR3 &vPosition, float size)
{
	float fx, fz;
	float nTile = static_cast<float>(m_nTile);

	fx =   vPosition.x / m_fScale  / m_nCell;
	fz = -(vPosition.z / m_fScale) / m_nCell;

	int startX	= fx - size / m_fScale / m_nCell;
	int startZ	= fz - size / m_fScale / m_nCell;
	int endX	= fx + size / m_fScale / m_nCell;
	int endZ	= fz + size / m_fScale / m_nCell;

	if(startX < 0)			startX	= 0;
	if(startZ < 0)			startZ	= 0;
	if(endX	> m_nTile)		endX	= m_nTile;
	if(endZ > m_nTile)		endZ	= m_nTile;

	//���� ���÷��� �귯�ô� ���� ������. �̰͸� �ϴ� �صѰž�

	for(int z = startZ; z <= endZ; z++)
	{
		for(int x = startX; x <= endX; x++)
		{
			int index = x + (z * m_nTile);

			bool bEqual = false;

			//D3DXVECTOR3 p		= m_pHeightMap[m_pTileInfo[index].nCenter].p;
			//if(Distance(vPosition, p) >= size)
			//	continue;//���� ��� ���� �ʾ����� �������� ��ŵ

			for( int i=0; i<m_vSplattingTiles.size(); ++i )
			{
				if( index == m_vSplattingTiles[i] )
				{
					bEqual = true;
					break;
				}
			}

			//����! ����!
			if( !bEqual )
				m_vSplattingTiles.push_back( index );
		}
	}
}

void AVTerrain::_CalcNormalVector(int x[3], int z[3])
{
	float left, right, bottom, top;
	D3DXVECTOR3 vNormal(0, 0, 0);

	int index;

	//�Ƹ� ������ ���° ���̰� �� �̷��� ��������ž�
	//0 tl 1 tr 2 bl�̰���

	for( int _z=z[0]+1; _z<z[2]-1; ++_z )
	{
		for( int _x=x[0]+1; _x<x[1]; ++_x )
		{
			index = _z*m_nSideVertex+_x;

			left = m_pHeightMap[ index - 1 ].p.y;    
			right = m_pHeightMap[ index + 1 ].p.y;   
			top  = m_pHeightMap[ index - m_nSideVertex ].p.y;
			bottom = m_pHeightMap[ index + m_nSideVertex ].p.y; 

			vNormal = D3DXVECTOR3( (left-right), 2, (bottom-top) );
			//�̰� ���� �Ƹ� �� ���� gpg3�ǿ� ����
			D3DXVec3Normalize( &vNormal, &vNormal ); // �������ͷ� ����

			m_pHeightMap[index].n = vNormal;   // ���� ����
		}
	}


}

int AVTerrain::SizeSplatTextures()
{
	return m_vSplatting.size();
}

void AVTerrain::SetSplattingMode(SPLATTING_OPTION option)
{ 
	m_splatOption = option; 
}