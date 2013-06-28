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
			//그냥 루프 돌려도 상관은 없는데, 나중에 못알아볼거 같다
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
	지형이 128 128이고 알파가 256 256이면, 알파 1에 지형 0.5 니까,
	2픽셀이 지형 1해당되도록 설정? 뭐 대충 머리굴리면 어떤건진 알거야
	*/
	float	pixSize		 = mapSize / static_cast<float>(m_alphaSize);
	int		outBrushSize = brushSize / pixSize ;
	int		inBrushSize	 = brushSize / pixSize / 3;

	// 값을 0~1.0으로 뽑기
	float tu =  vPickPos.x / mapSize;
	float tv = -vPickPos.z / mapSize;

	//tu, tv를 이용해서 알파텍스쳐 텍셀좌표를 구한다.
	//이게 128이면, 0~127쯤 구하겠지 뭐
	int texPosX = m_alphaSize * tu;
	int texPosY = m_alphaSize * tv;

	int startX	= 0, 
		startY	= 0,
		endX	= m_alphaSize, 
		endY	= m_alphaSize;
	//코드는 언제나 읽기 쉽게 ㅇㅇㅇ

	if((texPosX - outBrushSize) > 0)					startX	= texPosX - outBrushSize;
	if((texPosY - outBrushSize) > 0)					startY	= texPosY - outBrushSize;
	if((texPosX + outBrushSize) <= m_alphaSize)		endX	= texPosX + outBrushSize;
	if((texPosY + outBrushSize) <= m_alphaSize)		endY	= texPosY + outBrushSize;

	D3DLOCKED_RECT alphaTex_Locked;
	ZeroMemory(&alphaTex_Locked, sizeof(alphaTex_Locked));

	m_pAlphaTexture->LockRect(0, &alphaTex_Locked, NULL, 0);

	BYTE data;
	BYTE *defBits = static_cast<BYTE *>(alphaTex_Locked.pBits);

	//마우스 중심으로 구한 픽셀을 돌면서 값 세팅
	for( int y = startY; y < endY; y++ )
	{
		for( int x = startX; x < endX; x++ )
		{
//			D3DCOLOR_ARGB
			//4씩 움직이는게, ARGB로 이루어져 있잖아? 그거때문에
			int in = (alphaTex_Locked.Pitch * y) + (x * 4);

			//처음꺼면 A니까 알파값
			BYTE read = defBits[in];

			D3DXVECTOR3 distance;

			//알파텍스쳐의 마우스 피킹한 위치를 3D좌표로 바꾸고,
			//그 위치를 중심으로 안쪽, 바깥쪽원과의 거리를 계산한다.
			distance.x = (x * pixSize) - (texPosX * pixSize);
			distance.y = 0.f;
			distance.z = (m_alphaSize - y)*pixSize - (m_alphaSize - texPosY) * pixSize;

			float length = D3DXVec3Length( &distance );

			if( length <= inBrushSize )
				data = 0xff; //작은원안에 있으니까 모두 흰색
			else if( length <= outBrushSize )
			{
				length -= inBrushSize;	//작은 원과의 거리값
				int smooth = outBrushSize - inBrushSize;

				data = (smooth - length) / (float)smooth * 0xff;
			}
			else 
				continue;

			if(m_bAddMode)	read = (read < data ) ? data : read;
			else			read = ((read - data) < 0x00) ? 0x00 : (read - data);

			/*
			위에서 구한, 알파값만 저장하면 되긴하는데
			이거 그냥 그대로 그림 저장하면 검은색이잖아?
			그래서 원래 색도 바꿔주려고 하는거 뭐 확인용이니깐 ㅇㅇ..
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