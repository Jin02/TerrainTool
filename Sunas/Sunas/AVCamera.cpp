#include "AVCamera.h"


AVCamera::AVCamera(void)
{
	m_worldUp = D3DXVECTOR3(0, 1, 0);

	m_pos = D3DXVECTOR3(0, 0, 0);
	m_right = D3DXVECTOR3(1, 0, 0);
	m_look = D3DXVECTOR3(0, 0, 1.f);
	m_up = D3DXVECTOR3(0, 1, 0);
}

AVCamera::~AVCamera(void)
{
}

void AVCamera::strafe(float units)
{
	if( m_type == CAMERA_TYPE_LANDOBJECT )
		m_pos += D3DXVECTOR3(m_right.x, 0.f, m_right.z) * units;
	else //type == air
		m_pos += m_right * units;
}

void AVCamera::fly(float units)
{
	if( m_type == CAMERA_TYPE_LANDOBJECT ) return;

	//air type
	m_pos += m_up * units;
}

void AVCamera::walk(float units)
{
	if( units == 0 ) return;

	if(m_type == CAMERA_TYPE_LANDOBJECT)
	{
		D3DXVECTOR3 dir;
		D3DXVec3Cross(&dir, &m_right, &D3DXVECTOR3(0,1,0));
		m_pos += D3DXVECTOR3(dir.x, 0.f, dir.z) * units;
	}

	//type == air
	else
		m_pos += m_look * units;
}

void AVCamera::pitch(float angle)
{
	D3DXMATRIX T;
	D3DXMatrixRotationAxis(&T, &m_right, angle);

	D3DXVec3TransformCoord(&m_up, &m_up, &T);
	D3DXVec3TransformCoord(&m_look, &m_look, &T);
}

void AVCamera::yaw(float angle)
{
	D3DXMATRIX T;

//	if(m_type == CAMERA_TYPE_LANDOBJECT)
		D3DXMatrixRotationY(&T, angle);

	//else if(m_type == CAMERA_TYPE_AIRCRAFT)
	//	D3DXMatrixRotationAxis(&T, &m_up, angle);

	D3DXVec3TransformCoord(&m_right, &m_right, &T);
	D3DXVec3TransformCoord(&m_look, &m_look, &T);
}

void AVCamera::roll(float angle)
{
	if( m_type == CAMERA_TYPE_LANDOBJECT ) return;

	D3DXMATRIX T;

	D3DXMatrixRotationAxis(&T, &m_look, angle);

	D3DXVec3TransformCoord(&m_up, &m_up, &T);
	D3DXVec3TransformCoord(&m_right, &m_right, &T);
}

//void AVCamera::setViewMatrix(D3DXMATRIX *matrix)
//{
//	
//}

void AVCamera::getViewMatrix(D3DXMATRIX *matrix)
{
	D3DXVec3Normalize(&m_look, &m_look);

	D3DXVec3Cross(&m_up, &m_look, &m_right);
	D3DXVec3Normalize(&m_up, &m_up);

	D3DXVec3Cross(&m_right, &m_up, &m_look);
	D3DXVec3Normalize(&m_right, &m_right);

	float x = -D3DXVec3Dot(&m_right, &m_pos);
	float y = -D3DXVec3Dot(&m_up, &m_pos);
	float z = -D3DXVec3Dot(&m_look, &m_pos);

	(*matrix)(0, 0) = m_right.x;
	(*matrix)(0, 1) = m_up.x;
	(*matrix)(0, 2) = m_look.x;
	(*matrix)(0, 3) = 0.f;

	(*matrix)(1, 0) = m_right.y;
	(*matrix)(1, 1) = m_up.y;
	(*matrix)(1, 2) = m_look.y;
	(*matrix)(1, 3) = 0.f;

	(*matrix)(2, 0) = m_right.z;
	(*matrix)(2, 1) = m_up.z;
	(*matrix)(2, 2) = m_look.z;
	(*matrix)(2, 3) = 0.f;

	(*matrix)(3, 0) = x;
	(*matrix)(3, 1) = y;
	(*matrix)(3, 2) = z;
	(*matrix)(3, 3) = 1.f;
}

void AVCamera::setCameraType(CAMERA_TYPE type)
{
	m_type = type;
}

void AVCamera::setPosition(D3DXVECTOR3 &pos)
{
	m_pos = pos;
}

D3DXVECTOR3& AVCamera::getPosition()
{
	return m_pos;
}

D3DXVECTOR3& AVCamera::getRight()
{
	return m_right;
}

D3DXVECTOR3& AVCamera::getUp()
{
	return m_up;
}

D3DXVECTOR3& AVCamera::getLook()
{
	return m_look;
}

void AVCamera::setCamera(D3DXVECTOR3 &vEye, D3DXVECTOR3 &vLook)
{
	m_pos  = vEye;
	m_look = vLook;
}

AVCamera* AVCamera::GetCamera()
{
	if(_sharedCamera == NULL)
		_sharedCamera = new AVCamera;

	return _sharedCamera;
}

void AVCamera::deleteCamera()
{
	if(_sharedCamera) delete _sharedCamera;
	_sharedCamera = NULL;
}

void AVCamera::setDegree(float angleX, float angleY, float angleZ)
{
	pitch(D3DX_PI/180.f * angleX);
//	pitch(D3D
}