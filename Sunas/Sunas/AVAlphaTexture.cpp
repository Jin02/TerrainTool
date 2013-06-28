#include "AVAlphaTexture.h"
#include "Information.h"
AVAlphaTexture::AVAlphaTexture(LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_pd3dDevice	= pd3dDevice;
	m_bAddMode		= true;
}

AVAlphaTexture::~AVAlphaTexture(void)
{
	Destory();
}

bool AVAlphaTexture::Create(int alphaSize)
{
	alphaSize = nextPOT(alphaSize);
	
	if( FAILED( D3DXCreateTexture( m_pd3dDevice, alphaSize, alphaSize, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pAlphaTexture)))
		return false;

	D3DLOCKED_RECT alphaTex_Locked;
	ZeroMemory( &alphaTex_Locked, sizeof(alphaTex_Locked) );

	if( FAILED( m_pAlphaTexture->LockRect(0, &alphaTex_Locked, NULL, 0) ))
		return false;

	BYTE *defBits = (BYTE*)alphaTex_Locked.pBits;

	int i = 0;

	for(int y=0; y<alphaSize; y++)
	{
		for(int x=0; x<alphaSize; x++)
		{
			defBits[i++] = 0x00; //A
			defBits[i++] = 0x00; //B
			defBits[i++] = 0x00; //G
			defBits[i++] = 0x00; //R
			//�׳� ���� ������ ����� ���µ�, ���߿� ���˾ƺ��� ����
		}
	}

	if( FAILED( m_pAlphaTexture->UnlockRect(0)) )
		return false;

	m_alphaSize = alphaSize;

	return true;
}

void AVAlphaTexture::Draw(float mapSize, int brushSize, D3DXVECTOR3 &vPickPos)
{
	/*
	������ 128 128�̰� ���İ� 256 256�̸�, ���� 1�� ���� 0.5 �ϱ�,
	2�ȼ��� ���� 1�ش�ǵ��� ����? �� ���� �Ӹ������� ����� �˰ž�
	*/
	float	pixSize		 = mapSize / static_cast<float>(m_alphaSize);
	int		outBrushSize = brushSize / pixSize ;
	int		inBrushSize	 = brushSize / pixSize / 3;

	// ���� 0~1.0���� �̱�
	float tu =  vPickPos.x / mapSize;
	float tv = -vPickPos.z / mapSize;

	//tu, tv�� �̿��ؼ� �����ؽ��� �ؼ���ǥ�� ���Ѵ�.
	//�̰� 128�̸�, 0~127�� ���ϰ��� ��
	int texPosX = m_alphaSize * tu;
	int texPosY = m_alphaSize * tv;

	int startX	= 0, 
		startY	= 0,
		endX	= m_alphaSize, 
		endY	= m_alphaSize;
	//�ڵ�� ������ �б� ���� ������

	if((texPosX - outBrushSize) > 0)					startX	= texPosX - outBrushSize;
	if((texPosY - outBrushSize) > 0)					startY	= texPosY - outBrushSize;
	if((texPosX + outBrushSize) <= m_alphaSize)		endX	= texPosX + outBrushSize;
	if((texPosY + outBrushSize) <= m_alphaSize)		endY	= texPosY + outBrushSize;

	D3DLOCKED_RECT alphaTex_Locked;
	ZeroMemory(&alphaTex_Locked, sizeof(alphaTex_Locked));

	m_pAlphaTexture->LockRect(0, &alphaTex_Locked, NULL, 0);

	BYTE data;
	BYTE *defBits = static_cast<BYTE *>(alphaTex_Locked.pBits);

	//���콺 �߽����� ���� �ȼ��� ���鼭 �� ����
	for( int y = startY; y < endY; y++ )
	{
		for( int x = startX; x < endX; x++ )
		{
//			D3DCOLOR_ARGB
			//4�� �����̴°�, ARGB�� �̷���� ���ݾ�? �װŶ�����
			int in = (alphaTex_Locked.Pitch * y) + (x * 4);

			//ó������ A�ϱ� ���İ�
			BYTE read = defBits[in];

			D3DXVECTOR3 distance;

			//�����ؽ����� ���콺 ��ŷ�� ��ġ�� 3D��ǥ�� �ٲٰ�,
			//�� ��ġ�� �߽����� ����, �ٱ��ʿ����� �Ÿ��� ����Ѵ�.
			distance.x = (x * pixSize) - (texPosX * pixSize);
			distance.y = 0.f;
			distance.z = (m_alphaSize - y)*pixSize - (m_alphaSize - texPosY) * pixSize;

			float length = D3DXVec3Length( &distance );

			if( length <= inBrushSize )
				data = 0xff; //�������ȿ� �����ϱ� ��� ���
			else if( length <= outBrushSize )
			{
				length -= inBrushSize;	//���� ������ �Ÿ���
				int smooth = outBrushSize - inBrushSize;

				data = (smooth - length) / (float)smooth * 0xff;
			}
			else 
				continue;

			if(m_bAddMode)	read = (read < data ) ? data : read;
			else			read = ((read - data) < 0x00) ? 0x00 : (read - data);

			/*
			������ ����, ���İ��� �����ϸ� �Ǳ��ϴµ�
			�̰� �׳� �״�� �׸� �����ϸ� ���������ݾ�?
			�׷��� ���� ���� �ٲ��ַ��� �ϴ°� �� Ȯ�ο��̴ϱ� ����..
			*/

			for(int i=0; i<4; i++)
				defBits[in++] = read;
		}
	}

	m_pAlphaTexture->UnlockRect(0);
}

void AVAlphaTexture::SetAddMode(bool is)
{
	m_bAddMode = is;
}

LPDIRECT3DTEXTURE9 AVAlphaTexture::Texture()
{
	return m_pAlphaTexture;
}

void AVAlphaTexture::Destory()
{
	m_pAlphaTexture->Release();
	m_pAlphaTexture = NULL;
}

void AVAlphaTexture::Save(FILE *fp)
{
	D3DLOCKED_RECT alphaTex_Locked;
	ZeroMemory(&alphaTex_Locked, sizeof(alphaTex_Locked));

	m_pAlphaTexture->LockRect(0, &alphaTex_Locked, NULL, 0);

	BYTE data;
	BYTE *defBits = static_cast<BYTE *>(alphaTex_Locked.pBits);

//	fwrite(defBits, sizeof(BYTE), m_alphaSize * m_alphaSize, fp);

	int i = 0;

	for(int x = 0; x <  m_alphaSize; ++x)
	{
		for(int z=0; z<m_alphaSize; ++z)
		{
			fwrite(&defBits[i], sizeof(BYTE), 1, fp);
			i+=4;
		}
	}

	m_pAlphaTexture->UnlockRect(0);
}

void AVAlphaTexture::load(BYTE *bit)
{
	D3DLOCKED_RECT alphaTex_Locked;
	ZeroMemory(&alphaTex_Locked, sizeof(alphaTex_Locked));

	m_pAlphaTexture->LockRect(0, &alphaTex_Locked, NULL, 0);

	BYTE data;
	BYTE *defBits = static_cast<BYTE *>(alphaTex_Locked.pBits);

	int i = 0;
	int j = 0;

	for(int x = 0; x <  m_alphaSize; ++x)
	{
		for(int z=0; z<m_alphaSize; ++z)
		{
			defBits[i++] = bit[j];
			defBits[i++] = bit[j];
			defBits[i++] = bit[j];
			defBits[i++] = bit[j++];
		}
	}

	m_pAlphaTexture->UnlockRect(0);
}