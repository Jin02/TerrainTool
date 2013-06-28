#include "AVPicking.h"
#include "AVDirector.h"

AVPicking::AVPicking()
{
	m_pDevice = NULL;
	m_fDist = 0.f;
	m_vPick = D3DXVECTOR3(0, 0, 0);
}

AVPicking::~AVPicking(void)
{
}

AVPicking* AVPicking::GetPicking(LPDIRECT3DDEVICE9 pDevice)
{
	if( _sharedPicking == NULL )
		_sharedPicking = new AVPicking;

	_sharedPicking->m_pDevice = pDevice;

	return _sharedPicking;
}

void AVPicking::deletePickingMgr()
{
	if(_sharedPicking)
		delete _sharedPicking;

	_sharedPicking = NULL;
}

bool AVPicking::_raySphereIntersectionTest(AV_BOUNDING_SPHERE &sphere)
{
	D3DXVECTOR3 v = m_ray.origin - sphere.center;

	float b = 2.f * D3DXVec3Dot(&m_ray.direction, &v);
	float c = D3DXVec3Dot(&v, &v) - (sphere.radius * sphere.radius);

	float discriminant = (b * b) - (4.f * c);

	if( discriminant < 0.f )
		return false;

	discriminant = sqrtf(discriminant);

	float s0 = (-b + discriminant ) / 2.f;
	float s1 = (-b - discriminant ) / 2.f;

	if( s0 >= 0.f || s1 >= 0.f )
		return true;

	return false;
}

void AVPicking::_calcPickingRay()
{
	HWND hWnd = *(AVDirector::GetDiector()->GetApplication()->GetHwnd());

	float px = 0.f;
	float py = 0.f;
	float pz = 1.f;

	POINT point;
	GetCursorPos(&point);
	ScreenToClient(hWnd, &point);

	_setUpPicking();

	D3DVIEWPORT9 viewPort;
	m_pDevice->GetViewport(&viewPort);

	D3DXMATRIX proj;
	m_pDevice->GetTransform(D3DTS_PROJECTION, &proj);

	px = ((  ((2.f * (point.x - viewPort.X) / viewPort.Width ) - 1.0f)) - proj._31 ) / proj._11;
	py = ((- ((2.f * (point.y - viewPort.Y) / viewPort.Height) - 1.0f)) - proj._32 ) / proj._22;

	D3DXMATRIX view;
	m_pDevice->GetTransform(D3DTS_VIEW, &view);
	D3DXMatrixInverse(&view, 0, &view);

	m_ray.direction.x = px * view._11 + py * view._21 + pz * view._31;
	m_ray.direction.y = px * view._12 + py * view._22 + pz * view._32;
	m_ray.direction.z = px * view._13 + py * view._23 + pz * view._33;

	m_ray.origin.x    = view._41;
	m_ray.origin.y	  = view._42;
	m_ray.origin.z	  = view._43;

	D3DXMATRIX matWorld;
	m_pDevice->GetTransform( D3DTS_WORLD, &matWorld );
	D3DXMatrixInverse( &matWorld, NULL, &matWorld );
	D3DXVec3TransformCoord( &m_ray.direction, &m_ray.direction, &matWorld );
	D3DXVec3TransformCoord( &m_ray.origin, &m_ray.origin, &matWorld );
}

bool AVPicking::intersectSphere(AV_BOUNDING_SPHERE &object)
{
	_calcPickingRay();
	return _raySphereIntersectionTest(object);
}

//void AVPicking::_calcCorner(int *corner_4, int tileIndex, int nTile, int nCell)
//{
//	int x, y;
//	int nextTileLine;
//
//	x = tileIndex % nTile;
//	y = tileIndex / nTile;
//	nextTileLine = ((nTile * nCell) + 1) * nCell;
//
//	corner_4[0] = (y * nextTileLine) + (x * nCell);				//TL
//	corner_4[1] = corner_4[0] + nCell;							//TR
//	corner_4[2] = corner_4[0] + nextTileLine;					//BL
//	corner_4[3] = corner_4[2] + nCell;							//BR
//}
//

bool AVPicking::intersectTri(D3DXVECTOR3 *pv0, D3DXVECTOR3 *pv1, D3DXVECTOR3 *pv2, float &dist)
{
	static	HWND hWnd = *(AVDirector::GetDiector()->GetApplication()->GetHwnd());
	float		u, v;

	_calcPickingRay();

	if(D3DXIntersectTri(pv0, pv1, pv2, &m_ray.origin, &m_ray.direction, &u, &v, &dist)) 
	{
		m_vPick = *pv0 + u*(*pv1 - *pv0) + v*(*pv2 - *pv0);
		return true;
	}

	return false;
}

bool AVPicking::intersectAABB(AV_AABB &aabb, D3DXVECTOR3 &vPick)
{
	float t_min = 0.f;
	float t_max = pow(2.f, 32); //float == 2^32

	_calcPickingRay();

	for(int i=0; i<3; i++)
	{
		if(abs(m_ray.direction[i]) < AVPICKING_AABB_EPSILON)
		{
			if( m_ray.origin[i] < aabb.min[i] || m_ray.origin[i] > aabb.max[i] )
				return false;
		}
		else
		{
			float denom = 1.f / m_ray.direction[i];
			float t1 = (-m_ray.origin[i] - aabb.min[i]) * denom;
			float t2 = (-m_ray.origin[i] - aabb.max[i]) * denom;

			if(t1 > t2)
				SWAP(t1, t2);

			t_min = MAX(t_min, t1);
			t_max = MIN(t_max, t2);

			if( t_min > t_max )
				return false;
		}
	}

	vPick = m_ray.origin + t_min * m_ray.direction;

	return true;
}

void AVPicking::_setUpPicking()
{
	m_isPick = false;
	m_fDist = 10000000.f;
}

void AVPicking::raySort(float dist)
{
	if(m_fDist > dist)
	{
		if(dist > 0.f)
		{
			m_isPick	= true;
			m_fDist		= dist;
		}
	}
}