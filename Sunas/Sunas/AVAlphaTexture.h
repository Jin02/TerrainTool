#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>


class AVAlphaTexture
{
private:
	LPDIRECT3DDEVICE9		m_pd3dDevice;
	LPDIRECT3DTEXTURE9		m_pAlphaTexture;
	int						m_alphaSize;
public:
	bool					m_bAddMode;

public:
	AVAlphaTexture(LPDIRECT3DDEVICE9 pd3dDevice);
	~AVAlphaTexture(void);

	bool Create(int alphaSize);
	void Draw(float mapSize, int brushSize, D3DXVECTOR3 &vPickPos);
	void SetAddMode(bool is);
	LPDIRECT3DTEXTURE9 Texture();
	int alphaSize();

	void Destory();
	void Save(FILE *fp);
	void load(BYTE *bit);
};

