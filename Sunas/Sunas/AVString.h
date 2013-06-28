#pragma once

#include "AVBase.h"
#include "Information.h"
#include <d3d9.h>
#include <d3dx9.h>

#define AVFONT_DEFAULT_HEIGHT 20
#define AVFONT_DEFAULT_WEIGHT 400
#define AVFONT_DEFAULT_WIDTH  10

class AVString : public AVBase
{
	LPD3DXFONT		m_pFont;
	wchar_t		    m_content[256];
	RECT			m_rect;
	D3DCOLOR		m_color;

public:
	AVString(void);
	~AVString(void);

public:
	void CreateDefault(LPDIRECT3DDEVICE9 pDevice);
	void Destroy();

public:
	void SetFont(LPD3DXFONT pFont);
	void SetRect(AVRECT &rect);
	void SetColor(D3DCOLOR color);
	void SetText(wchar_t* str);

	LPD3DXFONT GetFont();
	AVRECT& GetRect();
	wchar_t* GetText();

public:
	virtual bool Update();
	virtual bool Render();
};

