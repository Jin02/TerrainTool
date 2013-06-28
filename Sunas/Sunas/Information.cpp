#include "Information.h"

ScaleInfo::ScaleInfo()
{
	x = y = 1.f;
}

ScaleInfo::~ScaleInfo()
{
}

ScaleInfo::ScaleInfo( float _x, float _y )
{
	x = _x;
	y = _y;
}

ScaleInfo& ScaleInfo:: operator+( const ScaleInfo& s)
{
	ScaleInfo temp(x+s.x, y+s.y);
	return temp;
}

ScaleInfo& ScaleInfo:: operator-( const ScaleInfo& s)
{
	ScaleInfo temp(x-s.x, y-s.y);
	return temp;
}

ScaleInfo& ScaleInfo:: operator/( const ScaleInfo& s)
{
	ScaleInfo temp(x/s.x, y/s.y);
	return temp;
}

ScaleInfo& ScaleInfo:: operator*( const ScaleInfo& s)
{
	ScaleInfo temp(x*s.x, y*s.y);
	return temp;
}

ScaleInfo& ScaleInfo:: operator+=( const ScaleInfo& s)
{
	return this->operator+(s);
}

ScaleInfo& ScaleInfo:: operator-=( const ScaleInfo& s)
{
	return this->operator-(s);
}

ScaleInfo& ScaleInfo:: operator/=( const ScaleInfo& s)
{
	return this->operator/(s);
}

ScaleInfo& ScaleInfo:: operator*=( const ScaleInfo& s)
{
	return this->operator*(s);
}

bool ScaleInfo:: operator==( const ScaleInfo& s)
{
	if( x == s.x && y == s.y ) return true;
	return false;
}

bool ScaleInfo:: operator!=( const ScaleInfo& s)
{
	return !(this->operator==(s));
}

/* GlobalFunc */

BOOL IsInRect( RECT* rc, POINT pt )
{
	if( ( rc->left <= pt.x ) && ( pt.x <= rc->right ) &&
		( rc->bottom <= pt.y ) && ( pt.y <= rc->top ) )
		return TRUE;

	return FALSE;
}

int	Log2( int n )
{
	for( int i = 1 ; i < 64 ; i++ )
	{
		n = n >> 1;
		if( n == 1 ) return i;
	}

	return 1;
}