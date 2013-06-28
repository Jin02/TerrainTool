#pragma once

#include "AVDirector.h"

class AVCamera
{
public:
	enum CAMERA_TYPE{ CAMERA_TYPE_LANDOBJECT, CAMERA_TYPE_AIRCRAFT };

private:
	CAMERA_TYPE			m_type;
	D3DXVECTOR3			m_right;	//same x
	D3DXVECTOR3			m_up;		//same y
	D3DXVECTOR3			m_look;		//same z
	D3DXVECTOR3			m_pos;
	D3DXVECTOR3			m_worldUp;

private:
	AVCamera(void);

public:
	~AVCamera(void);

	static AVCamera* GetCamera();
	void deleteCamera();

public:
	void strafe(float units);
	void fly(float units);
	void walk(float units);

	void pitch(float angle);
	void yaw(float angle);
	void roll(float angle);

	void getViewMatrix(D3DXMATRIX *matrix);
	

	void setCameraType(CAMERA_TYPE type);
	void setPosition(D3DXVECTOR3 &pos);
	void setDegree(float angleX, float angleY, float angleZ);

	D3DXVECTOR3& getPosition();
	D3DXVECTOR3& getRight();
	D3DXVECTOR3& getUp();
	D3DXVECTOR3& getLook();

	void setCamera(D3DXVECTOR3 &vEye, D3DXVECTOR3 &vLook);
};

static AVCamera *_sharedCamera = NULL;