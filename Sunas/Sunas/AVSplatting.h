#pragma once

#include "AVAlphaTexture.h"

class AVSplatting
{
private:
	int						m_id;
	int						m_texID;
	AVAlphaTexture			m_alpha;
	LPDIRECT3DTEXTURE9		m_pTex;
	LPDIRECT3DDEVICE9		m_pDevice;

public:
	AVSplatting(void);
	AVSplatting(LPDIRECT3DDEVICE9 pDevice);
	~AVSplatting(void);

	void Create(int alphaSize, LPDIRECT3DTEXTURE9 pTex, int id, int texID);
	LPDIRECT3DTEXTURE9 SplatTexture();
	LPDIRECT3DTEXTURE9 AlphaTexture();
	void AlphaMapDraw(float mapSize, int brushSize, D3DXVECTOR3 &vPickPos);
	void AlphaMapMode(bool isAddMode);
	int Id();
	int texID() { return m_texID; }
	LPDIRECT3DTEXTURE9& Texture() { return m_pTex; }
	void save(FILE *fp);
	void load(BYTE *bit);
};

