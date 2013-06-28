#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "Information.h"

//그림띄울려면.. 구조체정의부터 다시해야겠지?
//근데 sprite사용하면 안되나

class AVSTPreview
{
private:
	LPDIRECT3DDEVICE9       m_pd3dDevice;
	LPD3DXSPRITE			m_pSprite;
	AVRECT					m_rect;
	LPDIRECT3DTEXTURE9		m_tex;

public:
	AVSTPreview(void);
	~AVSTPreview(void);

	bool InitDevice(HWND hWnd, int w, int h);
	void SetPosition(int x, int y);
	void SetTexture(wchar_t *path);

private:
	void _Draw();

public:
	void Render();
	void Release();
};

