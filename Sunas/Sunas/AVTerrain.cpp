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

	//타일별로 계산하는 함수다

	int nextTileX = (index % m_nTile) * m_nCell;
	int nextTileZ = (index / m_nTile) * (m_nSideVertex * m_nCell);

	//알파텍스쳐 1장은 모든 지형에 적용되니깐, 이렇게 하는거야
	float alphaTu = (float)(index % m_nTile) / (float)m_nTile;
	float alphaTv = (float)(index / m_nTile) / (float)m_nTile;

	int i=0;
	int z=0;
	int x=0;
	for( z=0; z<m_nCell+1; ++z )
		for( x=0; x<m_nCell+1; ++x )
		{
			pTileInfo->pVertex[i]		= m_pHeightMap[ nextTileZ + nextTileX + (z * m_nSideVertex) + x ];
			pTileInfo->pVertex[i].t.x	= (float)x / (float)(m_nCell); // 타일별로 0.0 ~ 1.0으로 찍는다.
			pTileInfo->pVertex[i].t.y	= (float)z / (float)(m_nCell); // 텍스처 매트릭스를 이용해서 찍을것임.
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
	int level	= (int)pow( 4.f, LODLevel );		// 레벨마다 (1, 4, 16으로 4의 제곱으로 커져야 한다
	int size	= m_nTileIndices / level;	// 인덱스 사이즈(레벨마다 4의 제곱승으로 나눠야 한다)
	int next	= (int)pow( 2.f, LODLevel );		// 다음 버텍스로 넘어가는 크기

	// LOD 레벨별로 크랙 인덱스 버퍼을 만듭니다.
	int crackIndex=0;
	for( crackIndex=0; crackIndex<5; crackIndex++ )
	{
		// 0번째 레벨일 경우 크랙 인덱스 버퍼가 필요없으므로 패스
		if( LODLevel == 0 && crackIndex > 0 )
		{
			m_ppIB[LODLevel][crackIndex] = NULL;
			continue;
		}

		// 0은 기본 크랙 인덱스 버퍼
		int indexSize = size;
		if( 0 < LODLevel && (1 <= crackIndex && crackIndex < 5) )	// 1 ~ 4는 각각 cell당 3개의 삼각형 추가
		{
			// 셀의 가장자리 방향의 3개의 삼각형 추가(1삼각형당 3인덱스)
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

	//생각해보니까; 각각 타일의 정보도 필요할거 같고
	//전체 타일정보도 필요할거 같기도함

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
	//타일마다 vb제거

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
			//화면 정 가운데를 0,0도 맞추는것도 좋은데 나중에 계산할때 귀찮을듯
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
		// 위의 숫자처럼 left랑 비교해서 레벨이 틀릴경우 count=1
		// left, up 2개면 1+2=3이 된다. up, right 2개면 2+4=6 어떤수도 중복되지 않는다.
		// 즉 2개가 틀릴 경우는 그 위치에 해당하는 대각선 숫자가 나오게 된다.

		// left, up, right, down을 비교하면서 현재레벨이 주변레벨보다 클 경우만 적용한다.
		// 이유는 레벨이 클 수록 세부레벨로 나눠야 하기 때문이다.
		if( current <= cmp[i] )		// 현재레벨이 주변 레벨보다 작을 경우
			count += 0;
		else if( current > cmp[i] )	// 현재레벨이 주변레벨보다 클 경우
			count += (int)pow( 2.f, i );
	}

	// 0, 3, 6, 9, 12는 낮은 레벨이여서 
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


	//m_vVisibleTiles는 QuadTree에서 컬링되어
	//출력할수있는 값들을 나타낸다.
	//이걸로 slod의 크랙도 조절하고, 나타낼수있는 타일인 만큼
	//여기서 피킹질도 해야한다
	//일단 내일 일어나서 타일번호를 통한 실제 지형의 인덱스 번호들을 얻는
	//공식을 생각좀 해보자 있는거긴해도 생각하면 더 좋으니깐 해보자
	//여튼 그걸 얻으면 그 타일이랑 비교좀 때려야겠지 여튼 찾자


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
			//빼주는거에서도 뭐 해주긴 해야는데,
			//일단 냅두자 다른거 급한게 많아
		}
	}

	return true;
}

void AVTerrain::_BaseRender(int index)
{
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );	// 0번 텍스처 스테이지의 확대 필터
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );	// 0번 텍스처 스테이지의 축소 필터

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );		// 0번 텍스처 : 0번 텍스처 인덱스 사용
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );		// MODULATE로 섞는다.
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );		// 텍스처
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );		// 정점색
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

		// 기본 알파 옵션
		m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

		m_pd3dDevice->SetTexture( 0, m_vSplatting[i]->AlphaTexture() );
		m_pd3dDevice->SetTexture( 1, m_vSplatting[i]->SplatTexture() );
		m_pd3dDevice->SetStreamSource( 0, m_pTileInfo[index].pVB, 0, sizeof(TERRAIN_VERTEX) );
		m_pd3dDevice->SetIndices( m_ppIB[m_pTileInfo[index].nLodLevel][m_pTileInfo[index].nLodCrackIndex] );
		m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_nTileVertex, 0, m_pTileInfo[index].nTriangleNum );
	}

	// 멀티텍스쳐 셋팅을 사용을 중지 한다.
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DISABLE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	// 알파 사용을 중지 한다.
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
	x =  (x / m_fScale); //기준을, 타일형으로 바꿔준다
	z = -(z / m_fScale);

	//일단, 마우스가 맵 안에 있는지 확인해야해
	//피킹때 이 처리 안하긴 했는데, 여기서 처리 안한다면 버그투성이겠지

	if( x <= 0.f || z <= 0.f ||	x >= (m_nSideVertex - 1) ||	z >= (m_nSideVertex - 1) )
		return 0.f; //마우스 범위에 없는 구간들이니, 0.f를 리턴한다.

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

	float dx = x - nx; //는 소숫점 뜯기
	float dz = z - nz; //졸령 벌써 5시야
	float height = 0.f;

	//간단히 생각한다면 nx, nz는 큰 값들이잖아.
	//뭐 그걸로 지형 정점들 찾는데 썼다고 생각하면되
	//nx nz는 대략적인 값, dx dz는 세부적인 값이라고 생각하면 되려나
	//일단 nx nz로 대략적인 타일 얻고나서, dx dz는 세부적이니까
	//이걸로 높이를 찾으면되.
	//선형보간법 사용하면 될거야 어짜피 직선들이니깐 천천히 생각해봐

	//대충 사각형의 중점을 생각하면 이게 무슨 방식이구나 라고 생각이
	//들거야
	if(dz < 1.f - dx)  // 위쪽 삼각형
	{
		float uy = tileQuad[1] - tileQuad[0]; 
		float vy = tileQuad[2] - tileQuad[0];

		height = tileQuad[0] + Lerp(0.0f, uy, dx) + Lerp(0.0f, vy, dz);
	}
	else // 아래쪽 삼각형 DCB
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
	D3DXVECTOR3		newPos;//이어질 위치

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
	count /= 4; //사각형이니까,

	BRUSH_VERTEX	line[2];
	D3DXVECTOR3		curPos(1.f, 0.f, 0.f);
	D3DXVECTOR3		newPos;//이어질 위치

	D3DXMATRIX		mat;
	D3DXMatrixIdentity(&mat);

	line[1].p		= curPos * size + vPickPos;
	line[1].p.y		= GetHeight( line[1].p.x, line[1].p.z ) + 0.5f;
	line[1].color	= line[0].color = 0xffff0000;//D3DCOLOR_ARGB(255,255,0,0);


	D3DXVECTOR3 vTL( -1.0f, 0.0f, +1.0f );
	D3DXVECTOR3 vTR( +1.0f, 0.0f, +1.0f );
	D3DXVECTOR3 vBL( -1.0f, 0.0f, -1.0f );
	D3DXVECTOR3 vBR( +1.0f, 0.0f, -1.0f );
	//1씩 넣은 이유는 별거 업고 그냥 스칼라값 곱해 넣으려는데
	//일일히 쓰긴 귀찮잖아 아 나도 몰마 ㅇㄹ무닝ㄹ 졸령 지금은 9시

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

		// p(t) = (1-t)p1 +(t)p2 는 cg책 4단원에 잇던거..
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
		return; //마우스 범위에 없는 구간들이니, 0.f를 리턴한다.

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

	//노멀 계산해야되..
	//음.. 그럼.. 음.. ㄱㄱ
	int x[3], z[3];
	x[0] = z[0] = m_pTileInfo[ startX ].nCorner[0];
	x[1] = z[1] = m_pTileInfo[ endX	  ].nCorner[1];
	x[2] = z[2] = m_pTileInfo[ endZ	  ].nCorner[2];

	//sv로 나누면, x랑 z에 그게 들어가
	//그니까 이게 뭐냐면, 음.. 조사해서 넣어야할게 현재 선택된
	//현재 타일에서 tl, tr, bl을 조사해야하잖아
	//그럼 그러니까 그렇게 하면되
	//그러니까 x에서 1이 나오잖아. 그럼 아래에서 걸려지는건 다시 1이되겠지
	//아으 뭐라 설명해야할까, 
	//아 그래. 한줄 한줄이라고 생각할때, 저 아래 수식을 계산하면
	//마지막꺼, 몇번째줄꺼, 뭐 이런식 가로 몇줄 세로 몇줄 이런거
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
	// 좌표를 벗어나면 타일번호를 구하지 않는다.
	if( vPosition.x < 0 || vPosition.z > 0 )
		return -1;

	// 현재 위치의 타일 번호를 구한다. 
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

	//지금 스플래팅 브러시는 원만 존재해. 이것만 일단 해둘거야

	for(int z = startZ; z <= endZ; z++)
	{
		for(int x = startX; x <= endX; x++)
		{
			int index = x + (z * m_nTile);

			bool bEqual = false;

			//D3DXVECTOR3 p		= m_pHeightMap[m_pTileInfo[index].nCenter].p;
			//if(Distance(vPosition, p) >= size)
			//	continue;//원에 들어 오지 않았으면 다음으로 스킵

			for( int i=0; i<m_vSplattingTiles.size(); ++i )
			{
				if( index == m_vSplattingTiles[i] )
				{
					bEqual = true;
					break;
				}
			}

			//새거! 업어!
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

	//아마 위에서 몇번째 줄이고 뭐 이런거 계산했을거야
	//0 tl 1 tr 2 bl이겠지

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
			//이거 식이 아마 그 뭐냐 gpg3권에 잇음
			D3DXVec3Normalize( &vNormal, &vNormal ); // 단위벡터로 만듬

			m_pHeightMap[index].n = vNormal;   // 법선 계산끗
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