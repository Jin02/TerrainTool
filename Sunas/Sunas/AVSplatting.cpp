#include "AVSplatting.h"


AVSplatting::AVSplatting(void)
	: m_alpha(NULL)
{
	m_pDevice = NULL;
	m_texID = -1;
}

AVSplatting::AVSplatting(LPDIRECT3DDEVICE9 pDevice)
	: m_alpha(pDevice)
{
	m_pDevice	= pDevice;
	m_id		= -1;
}

AVSplatting::~AVSplatting(void)
{
	m_alpha.Destory();
}

void AVSplatting::Create(int alphaSize, LPDIRECT3DTEXTURE9 pTex, int id, int texID)
{
	m_alpha.Create(alphaSize);
	m_id = id;
	m_pTex = pTex;
	m_texID = texID;
}

LPDIRECT3DTEXTURE9 AVSplatting::SplatTexture()
{
	return m_pTex;
}

LPDIRECT3DTEXTURE9 AVSplatting::AlphaTexture()
{
	return m_alpha.Texture();
}

void AVSplatting::AlphaMapDraw(float mapSize, int brushSize, D3DXVECTOR3 &vPickPos)
{
	m_alpha.Draw(mapSize, brushSize, vPickPos);
}

void AVSplatting::AlphaMapMode(bool isAddMode)
{
	m_alpha.SetAddMode(isAddMode);
}

int AVSplatting::Id()
{
	return m_id;
}

void AVSplatting::save(FILE *fp)
{
	m_alpha.Save(fp);
}

void AVSplatting::load(BYTE *bit)
{
	m_alpha.load(bit);
}