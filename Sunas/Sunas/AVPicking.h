#pragma once

#include "Information.h"
#include <vector>

#define AVPICKING_AABB_EPSILON 10.f

class AVPicking
{
private:
	LPDIRECT3DDEVICE9	m_pDevice;
	D3DXVECTOR2			m_vMouse;
	AV_RAY				m_ray;
	float				m_fDist;
	bool				m_isPick;
	D3DXVECTOR3			m_vPick;

private:
	AVPicking(void);

public:
	~AVPicking(void);
	static AVPicking* GetPicking(LPDIRECT3DDEVICE9 pDevice);

	void deletePickingMgr();


private:
	bool	_raySphereIntersectionTest(AV_BOUNDING_SPHERE &sphere);
	
private:
	void	_setUpPicking();
	void	_calcPickingRay();
	//void	_calcCorner(int *corner_4, int tileIndex, int nTile, int nCell);
	//int		_calcCenter(int tileIndex, int nTile, int nCell);

public:
	bool			intersectSphere(AV_BOUNDING_SPHERE &object);
	bool			intersectTri(D3DXVECTOR3 *p0, D3DXVECTOR3 *p1, D3DXVECTOR3 *p2, float &dist);
	bool			intersectAABB(AV_AABB &aabb, D3DXVECTOR3 &vPick);
	void			raySort(float dist);

	D3DXVECTOR3&	GetPickPos() { return m_vPick; }
};

static AVPicking *_sharedPicking = NULL;