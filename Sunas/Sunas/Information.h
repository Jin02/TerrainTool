#pragma once

#pragma warning (disable:4172)

#include <d3d9.h>
#include <d3dx9.h>

#define AV_MOUSE_LEFT		0
#define AV_MOUSE_RIGHT		1

#define AV_MOUSE_DOWN		1
#define AV_MOUSE_UP			2

#define MAX_TERRAIN_TEX 4
#define USE_INDEX_16

typedef struct AVRECT{
	float x,y,w,h;

	AVRECT(float _x = 0, float _y = 0, float _w = 0, float _h = 0)
	{
		x = _x;
		y = _y;
		w = _w;
		h = _h;
	}

}AVRECT;

class ScaleInfo
{
public:
	float x;
	float y;

	ScaleInfo();
	ScaleInfo( float _x, float _y );

	~ScaleInfo();

	ScaleInfo& operator += ( const ScaleInfo& s);
	ScaleInfo& operator -= ( const ScaleInfo& s);
	ScaleInfo& operator /= ( const ScaleInfo& s);
	ScaleInfo& operator *= ( const ScaleInfo& s);

	ScaleInfo& operator + ( const ScaleInfo& s);
	ScaleInfo& operator - ( const ScaleInfo& s);
	ScaleInfo& operator / ( const ScaleInfo& s);
	ScaleInfo& operator * ( const ScaleInfo& s);
	
	bool operator == ( const ScaleInfo& s);
	bool operator != ( const ScaleInfo& s);

};

typedef struct TERRAIN_VERTEX
{
	enum _FVF { FVF=(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2) };

	D3DXVECTOR3	p;
	D3DXVECTOR3	n;
	D3DXVECTOR2	t;
	D3DXVECTOR2 t2;

}TERRAIN_VERTEX;

typedef struct BRUSH_VERTEX
{
	enum _FVF { FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE };

	D3DXVECTOR3		p;
	DWORD			color;

}BRUSH_VERTEX;

struct TRIANGLE_INDEX
{
#ifdef _USE_INDEX16
	WORD _0, _1, _2;
#else
	DWORD	_0, _1, _2;
#endif
};

BOOL IsInRect( RECT* rc, POINT pt );
int	Log2( int n );

struct AV_RAY
{
	D3DXVECTOR3 origin;
	D3DXVECTOR3 direction;

	AV_RAY(void)
	{
		origin	  = D3DXVECTOR3(0, 0, 0);
		direction = D3DXVECTOR3(0, 0, 0);
	}

	AV_RAY(D3DXVECTOR3 &_origin, D3DXVECTOR3 &_direction)
	{
		origin		= _origin;
		direction	= _direction;
	}
};

struct AV_BOUNDING_SPHERE
{
	D3DXVECTOR3 center;
	float		radius;

	AV_BOUNDING_SPHERE(void)
	{
		center = D3DXVECTOR3(0, 0, 0);
		radius = 0;
	}

	AV_BOUNDING_SPHERE(D3DXVECTOR3 &_center, float &_radius)
	{
		center = _center;
		radius = _radius;
	}
};

struct AV_AABB
{
	D3DXVECTOR3 min;
	D3DXVECTOR3 max;

	AV_AABB()
	{
		min = D3DXVECTOR3(0, 0, 0);
		max = D3DXVECTOR3(0, 0, 0);
	}

	AV_AABB(D3DXVECTOR3 &_min, D3DXVECTOR3 &_max)
	{
		min = _min;
		max = _max;
	}
};

template<typename T>
inline void SAFE_DELETE(T& p){if(p){delete p; p=NULL;}}

template<typename T>
inline void SAFE_ARRARY_DELETE(T& p){if(p){delete[] p; p=NULL;}}

template<typename T>
inline void SAFE_RELEASE(T& p){if(p){p->Release(); p=NULL;}}

template<typename T>
inline T MIN(T a, T b){ return a < b ? a : b;}

template<typename T>
inline T MAX(T a, T b){ return a > b ? a : b;}

template<typename T>
inline void SWAP(T &a, T &b)
{
	T temp;

	temp = a;
	a    = b;
	b    = temp;
}

inline float Lerp(float a, float b, float t)
{
	return a - (a*t) + (b*t);
}

typedef enum BRUSH_TYPE
{
	BRUSH_TYPE_NONE,
	BRUSH_TYPE_CIRCLE,
	BRUSH_TYPE_SQUARE
}BRUSH_TYPE;

inline float Distance(D3DXVECTOR3 &v1, D3DXVECTOR3 &v2)
{
	return sqrtf( pow(v2.x - v1.x, 2) + pow(v2.y - v1.y, 2) + pow(v2.z - v1.z, 2) );
}

inline int nextPOT(int x)
{
	x = x - 1;
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);

	return x + 1;
}