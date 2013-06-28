#include "AVString.h"


AVString::AVString(void)
{
	m_color = D3DCOLOR_ARGB(255, 255, 255, 255);
	ZeroMemory(m_content, sizeof(wchar_t) * 256);
	
	RECT rect = {0, 0, -1, -1};
	m_rect = rect;
}

AVString::~AVString(void)
{
	Destroy();
}

void AVString::SetFont(LPD3DXFONT pFont)
{
	m_pFont = pFont;
}

void AVString::SetText(wchar_t* str)
{	
	wcscpy(m_content, str);
}

wchar_t* AVString::GetText()
{ 
	return m_content;		
}

void AVString::SetRect(AVRECT &rect)
{
	m_rect.left		= (long)(rect.x);
	m_rect.top		= (long)(rect.y);
	m_rect.right	= (long)(rect.x + rect.w);
	m_rect.bottom	= (long)(rect.y + rect.h);
}

void AVString::SetColor(D3DCOLOR color)
{
	m_color = color;
}

AVRECT& AVString::GetRect()
{ 
	return AVRECT( m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
}

void AVString::CreateDefault(LPDIRECT3DDEVICE9 pDevice)
{
	D3DXFONT_DESC info;

	ZeroMemory(&info, sizeof(D3DXFONT_DESC));

	info.Height			 = AVFONT_DEFAULT_HEIGHT;
	info.Weight			 = AVFONT_DEFAULT_WEIGHT;
	info.Width			 = AVFONT_DEFAULT_WIDTH;
	info.OutputPrecision = OUT_DEFAULT_PRECIS;
	info.Quality		 = DEFAULT_QUALITY;
	info.PitchAndFamily  = DEFAULT_PITCH;
	info.Italic			 = false;
	info.CharSet		 = DEFAULT_CHARSET;
	info.MipLevels		 = 1;
	wcscpy(info.FaceName, L"Times New Roman");

	D3DXCreateFontIndirect(pDevice, &info, &m_pFont);
}

void AVString::Destroy()
{
	if(m_pFont != NULL)
	{
		m_pFont->Release();
		m_pFont = NULL;
	}
}

bool AVString::Update()
{
	return true;
}

bool AVString::Render()
{
	m_pFont->DrawText(NULL, m_content, -1, &m_rect, D3DFMT_A8R8G8B8,m_color);
	return true;
}

LPD3DXFONT AVString::GetFont()
{
	return m_pFont;
}