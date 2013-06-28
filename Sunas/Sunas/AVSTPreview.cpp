#include "AVSTPreview.h"
#include "AVDirector.h"

AVSTPreview::AVSTPreview(void)
{
	m_pd3dDevice	= NULL;
	m_pSprite		= NULL;
	ZeroMemory(&m_rect, sizeof(AVRECT));
	m_tex			= NULL;
}

AVSTPreview::~AVSTPreview(void)
{
	Release();
}

bool AVSTPreview::InitDevice(HWND hWnd, int w, int h)
{
	SAFE_RELEASE(m_tex);
	SAFE_RELEASE(m_pSprite);
	SAFE_RELEASE(m_pd3dDevice);

	AVWinD3D *d3d = AVDirector::GetDiector()->GetApplication();
	D3DPRESENT_PARAMETERS	d3dpp;
	
	D3DDISPLAYMODE d3ddm;
	if( FAILED( d3d->GetD3D()->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )	
	{
		return E_FAIL;
	}

	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));

	d3dpp.BackBufferCount			= 1;
    d3dpp.Windowed					= TRUE;
    d3dpp.SwapEffect				= D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferWidth			= w;
    d3dpp.BackBufferHeight			= h;
    d3dpp.BackBufferFormat			= d3ddm.Format;
	d3dpp.hDeviceWindow				= hWnd;
	d3dpp.EnableAutoDepthStencil	= TRUE;
	d3dpp.AutoDepthStencilFormat	= D3DFMT_D24S8;
	m_rect.w = w;
	m_rect.h = h;

	if( FAILED( d3d->GetD3D()->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &m_pd3dDevice ) ) )
	{
		return false;
	}
	
	if( FAILED( D3DXCreateSprite(m_pd3dDevice, &m_pSprite)))
	{
		return false;
	}

	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,  FALSE );

	return true;
}

void AVSTPreview::_Draw()
{
	RECT rect;
	rect.left	= m_rect.x;
	rect.top	= m_rect.y;
	rect.right	= m_rect.x + m_rect.w;
	rect.bottom	= m_rect.y + m_rect.h;

	m_pSprite->Draw(m_tex, &rect, NULL, &D3DXVECTOR3(0, 0, 0), 0xffffffff);
}

void AVSTPreview::SetPosition(int x, int y)
{
	m_rect.x = x;
	m_rect.y = y;
}

void AVSTPreview::SetTexture(wchar_t *path)
{
	SAFE_RELEASE(m_tex);
	D3DXCreateTextureFromFileEx(m_pd3dDevice, path, D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE, D3DCOLOR_ARGB(0,0,0,255), NULL, NULL, &m_tex);
}

void AVSTPreview::Render()
{
	if(m_pd3dDevice == NULL) return;

	m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0L );
	if( SUCCEEDED(m_pd3dDevice->BeginScene()) )
	{
		m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);

		_Draw();

		m_pSprite->End();
		m_pd3dDevice->EndScene();
	}

	m_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

void AVSTPreview::Release()
{
	SAFE_RELEASE(m_tex);
	SAFE_RELEASE(m_pSprite);
	SAFE_RELEASE(m_pd3dDevice);
}
