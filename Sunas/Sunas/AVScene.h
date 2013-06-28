#pragma once

#include "AVBase.h"
#include "AVWinD3D.h"

class AVScene : public AVBase
{
public:
	AVScene(void* data = NULL);
	~AVScene(void);

	virtual void Init()		= 0;
	virtual void Loop()		= 0;
	virtual	void End()		= 0;
	virtual void Mouse(D3DXVECTOR2 &vPosition, int Left, int Right)	= 0;
	virtual void KeyBoard(bool *push, bool *up) = 0;

	void ChangeScene(AVScene* scene, bool isDelete);
};