#include "AVWinD3D.h"

AVWinD3D::AVWinD3D(WNDPROC wndProc, HINSTANCE hInst, AVRECT &windowRect, HWND parentHandle, wchar_t *pWindowName, bool isChild, wchar_t *menuName)
:AVWindow(windowRect, hInst, isChild, pWindowName, parentHandle, wndProc, menuName)
{	
    ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));

	m_d3dpp.Windowed				= TRUE;
	m_d3dpp.PresentationInterval    = D3DPRESENT_INTERVAL_IMMEDIATE;
	m_d3dpp.BackBufferCount         = 1;
    m_d3dpp.SwapEffect				= D3DSWAPEFFECT_DISCARD;
    m_d3dpp.BackBufferFormat		= D3DFMT_A8R8G8B8;
    m_d3dpp.EnableAutoDepthStencil	= TRUE;
    m_d3dpp.AutoDepthStencilFormat	= D3DFMT_D16;
}

AVWinD3D::~AVWinD3D(void)
{
//	UnregisterClass( m_applicationName, m_window->hInstance );

	SAFE_RELEASE(m_pD3D);
	SAFE_RELEASE(m_pd3dDevice);
}

void AVWinD3D::InitWinD3D(wchar_t *application, bool zBuffer, bool lighting, DWORD cullModeValue)
{
	Create();

   /* D3D */
	m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

	m_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&m_d3dpp, &m_pd3dDevice);


	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, cullModeValue );
	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, lighting );
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,  zBuffer );
	/* D3D */

	Show();
}