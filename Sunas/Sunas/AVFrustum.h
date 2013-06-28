#pragma once

#include <d3d9.h>
#include <d3dx9.h>

#define PLANE_EPSILON 0.f

class AVFrustum
{
private:
	D3DXVECTOR3			m_planeVertex[8]; //프로스텀 정점들
	D3DXVECTOR3			m_vPosition;
	D3DXPLANE			m_plane[6];		  //프로스텀 평면들

public:
	AVFrustum();
	~AVFrustum(void);

public:
	bool Make( D3DXMATRIXA16 *viewProjection );
	bool IsIn( D3DXVECTOR3 *v );
	bool IsInSphere(D3DXVECTOR3 *v, float radius);
	D3DXVECTOR3* GetPosition();
};

