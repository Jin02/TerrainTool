#include "AVFrustum.h"


AVFrustum::AVFrustum()
{
	ZeroMemory(&m_planeVertex[0], sizeof(D3DXVECTOR3) * 8);
	ZeroMemory(&m_plane[0], sizeof(D3DXPLANE) * 6);
	m_vPosition = D3DXVECTOR3(0,0,0);
}

AVFrustum::~AVFrustum(void)
{
}

bool AVFrustum::Make( D3DXMATRIXA16 *viewProjection )
{
	int				i;
	D3DXMATRIXA16	matInv;

	m_planeVertex[0].x = -1.0f;	m_planeVertex[0].y = -1.0f;	m_planeVertex[0].z = 0.0f;
	m_planeVertex[1].x =  1.0f;	m_planeVertex[1].y = -1.0f;	m_planeVertex[1].z = 0.0f;
	m_planeVertex[2].x =  1.0f;	m_planeVertex[2].y = -1.0f;	m_planeVertex[2].z = 1.0f;
	m_planeVertex[3].x = -1.0f;	m_planeVertex[3].y = -1.0f;	m_planeVertex[3].z = 1.0f;
	m_planeVertex[4].x = -1.0f;	m_planeVertex[4].y =  1.0f;	m_planeVertex[4].z = 0.0f;
	m_planeVertex[5].x =  1.0f;	m_planeVertex[5].y =  1.0f;	m_planeVertex[5].z = 0.0f;
	m_planeVertex[6].x =  1.0f;	m_planeVertex[6].y =  1.0f;	m_planeVertex[6].z = 1.0f;
	m_planeVertex[7].x = -1.0f;	m_planeVertex[7].y =  1.0f;	m_planeVertex[7].z = 1.0f;

	D3DXMatrixInverse(&matInv, NULL, viewProjection );

	for( i = 0; i < 8; i++ )
		D3DXVec3TransformCoord( &m_planeVertex[i], &m_planeVertex[i], &matInv );

	m_vPosition = ( m_planeVertex[0] + m_planeVertex[5] ) / 2.0f;
	
	// 얻어진 월드좌표로 프러스텀 평면을 만든다
	// 벡터가 프러스텀 안쪽에서 바깥쪽으로 나가는 평면들이다.
	D3DXPlaneFromPoints(&m_plane[0], m_planeVertex+4, m_planeVertex+7, m_planeVertex+6);	// 상 평면(top)
	D3DXPlaneFromPoints(&m_plane[1], m_planeVertex  , m_planeVertex+1, m_planeVertex+2);	// 하 평면(bottom)
	D3DXPlaneFromPoints(&m_plane[2], m_planeVertex  , m_planeVertex+4, m_planeVertex+5);	// 근 평면(near)
	D3DXPlaneFromPoints(&m_plane[3], m_planeVertex+2, m_planeVertex+6, m_planeVertex+7);	// 원 평면(far)
	D3DXPlaneFromPoints(&m_plane[4], m_planeVertex  , m_planeVertex+3, m_planeVertex+7);	// 좌 평면(left)
	D3DXPlaneFromPoints(&m_plane[5], m_planeVertex+1, m_planeVertex+5, m_planeVertex+6);	// 우 평면(right)

	return TRUE;
}

bool AVFrustum::IsIn( D3DXVECTOR3 *v )
{
	float		fDist;
	for(int i = 0 ; i < 6 ; i++ )
	{
		fDist = D3DXPlaneDotCoord( &m_plane[i], v );
		if( fDist > PLANE_EPSILON ) return false;
	}

	return TRUE;
}

bool AVFrustum::IsInSphere(D3DXVECTOR3 *v, float radius)
{
	float		fDist;

	for(int i=0; i<6; i++)
	{
		fDist = D3DXPlaneDotCoord( &m_plane[i], v );
		if( fDist > (radius + PLANE_EPSILON) ) return false;
	}

	return TRUE;
}

D3DXVECTOR3* AVFrustum::GetPosition()
{
	return &m_vPosition;
}