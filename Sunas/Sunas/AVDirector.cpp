#include "AVDirector.h"
#include "AVCamera.h"
#include "SharedTexture.h"

AVDirector::AVDirector(void)
{
	m_Scene			= NULL;
	m_bIsFrameCheck = true;
	m_fFPS			= 0.f;
	m_isStart		= false;
	m_state			= AV_STATE_INIT;
}

AVDirector::~AVDirector(void)
{
}

AVDirector* AVDirector::GetDiector()
{
	if( _sharedDirector == NULL )
	{
		_sharedDirector = new AVDirector;
	}

	return _sharedDirector;
}

void AVDirector::deleteDirector()
{
	SCENE->End();
	SAFE_DELETE(m_Scene);
	SAFE_DELETE(m_Application);

	if(_sharedDirector) delete _sharedDirector;
	_sharedDirector = NULL;

//	PostQuitMessage(0);
}

void AVDirector::RunApplication(float fCullValue, HINSTANCE hIns, HWND parentHandle, bool isChild, AVRECT &windowRect, wchar_t *application)
{

	m_Application = new AVWinD3D((WNDPROC)AVDirector::WndProc, hIns, windowRect, parentHandle, application, isChild);
	m_Application->InitWinD3D(application, true, true, D3DCULL_CCW);

	MSG msg;
	ZeroMemory( &msg, sizeof( msg ) );

	_initMatrix(fCullValue);
	_initLight();
	SetFog(fCullValue - 100.f, fCullValue - 50.f);

	if(m_Application->isChild())
		return;

	while( msg.message != WM_QUIT )
	{
		if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			RunToInnerSystem();
			tickTime();

			if(m_bIsFrameCheck)
				calculateFPS();
		}
	}
}

void AVDirector::RunToInnerSystem()
{
	static LPDIRECT3DDEVICE9 device = m_Application->GetD3DDevice();

    device->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(200,200,200), 1.0f, 0 );

	if( m_isStart )
	{
		if( m_state == AV_STATE_END )			
			SCENE->End();
		if( m_state == AV_STATE_INIT )		
			SCENE->Init(); 

		if( m_state != AV_STATE_LOOP )
		{
			m_state = (m_state + 1) % 3;
			return;
		}

		SCENE->Loop();
	}

	if( SUCCEEDED( device->BeginScene() ) )
	{
		//이너 업데이트에서 랜더부분, update부분 따로 뜯는걸 찾아보든가
		//솔까 루프 2번돌면 귀찮은데 이대로 하자
		SCENE->InnerUpdate();
		device->EndScene();
	}

	m_Application->GetD3DDevice()->Present(NULL,NULL,NULL,NULL);
}

void AVDirector::changeToState(const AVState state)
{
	m_state = state;
}

void AVDirector::tickTime()
{
	static float	lastTime;
	float			now;
	static DWORD	staticTime = GetTickCount();

	now = (double)(GetTickCount() - staticTime) / 1000;

	m_tickTime = now - lastTime;
	m_tickTime = MAX(0.f, m_tickTime);

	lastTime = now;
}

LRESULT WINAPI AVDirector::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#define LEFT  AV_MOUSE_LEFT
#define RIGHT AV_MOUSE_RIGHT


	AVScene *scene = AVDirector::GetDiector()->GetScene();

	bool UpKey[256] = {false, }, DownKey[256] = {false, };
	int Mouse[2] = {0, };

	if(AVDirector::GetDiector()->m_state != AV_STATE_LOOP)
		return DefWindowProc( hWnd, msg, wParam, lParam );

	switch( msg )
	{
	case WM_KEYDOWN:
		DownKey[wParam] = true;
		PROC_SCENE->KeyBoard(DownKey, UpKey);
		break;

	case WM_KEYUP:
		UpKey[wParam]  = true;
		PROC_SCENE->KeyBoard(DownKey, UpKey);
		break;		

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		Mouse[LEFT] = msg - WM_LBUTTONDOWN + 1;
		break;

	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		Mouse[RIGHT] = msg - WM_RBUTTONDOWN + 1;
		break;

	case WM_DESTROY:
		AVDirector::GetDiector()->deleteDirector();
		SharedTexture::GetSharedTexture()->Delete();
		return 0;
	}

	if( WM_MOUSEMOVE <= msg && msg <= WM_RBUTTONUP )
	{
		PROC_SCENE->Mouse(D3DXVECTOR2(LOWORD(lParam), HIWORD(lParam)),
			Mouse[LEFT], Mouse[RIGHT]);
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

AVWinD3D* AVDirector::GetApplication()
{ 
	return m_Application;	
}

float AVDirector::GetTickTime()
{ 
	return m_tickTime;		
}

int	AVDirector::GetState()
{ 
	return m_state;		
}

AVScene* AVDirector::GetScene()
{ 
	return m_Scene;		
}

void AVDirector::SetScene(AVScene* scene)
{ 
	m_Scene = scene;		
}

void AVDirector::_initMatrix(float fCull)
{
	LPDIRECT3DDEVICE9 device = m_Application->GetD3DDevice();
	D3DXMATRIXA16 matWorld;

	D3DXMatrixIdentity( &matWorld );
	device->SetTransform( D3DTS_WORLD, &matWorld );

    /// 뷰 행렬을 설정
   /* D3DXVECTOR3 vEyePt( 0.0f, 50.0f, (float)-30.0f );
    D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
    D3DXMatrixLookAtLH( &m_matView, &vEyePt, &vLookatPt, &vUpVec );
    device->SetTransform( D3DTS_VIEW, &m_matView );*/

	float w = m_Application->GetWindowSize().w;
	float h = m_Application->GetWindowSize().h;

    /// 실제 프로젝션 행렬
	D3DXMatrixPerspectiveFovLH( &m_matProj, D3DX_PI/4, w/h, 1.0f, fCull + PROJECTION_GAP );
    device->SetTransform( D3DTS_PROJECTION, &m_matProj );

	/// 프러스텀 컬링용 프로젝션 행렬
    D3DXMatrixPerspectiveFovLH( &m_matProj, D3DX_PI/4, w / h, 1.0f, fCull);

	/// 카메라 초기화
//	AVCamera::GetCamera()->setCamera( vEyePt, vLookatPt );
//	m_camera->setCamera( vEyePt, vLookatPt );
}

void AVDirector::_initLight()
{
	LPDIRECT3DDEVICE9 pDevice = m_Application->GetD3DDevice();
	D3DMATERIAL9 mtrl;	
	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
	mtrl.Ambient	= D3DXCOLOR(1.0f,1.0f,1.0f,1.0f);
	mtrl.Diffuse	= D3DXCOLOR(1.0f,1.0f,1.0f,1.0f);
	mtrl.Specular	= D3DXCOLOR(1.0f,1.0f,1.0f,1.0f);
	pDevice->SetMaterial( &mtrl );

	D3DLIGHT9 light;
	ZeroMemory( &light, sizeof(D3DLIGHT9) );
	light.Type		= D3DLIGHT_DIRECTIONAL;
	light.Diffuse	= D3DXCOLOR(1.0f,1.0f,0.0f,1.0f);

	D3DXVec3Normalize( (D3DXVECTOR3 *)&light.Direction, &D3DXVECTOR3(1.0f, -1.0f, 1.0f) );
	pDevice->SetLight( 0, &light );
	pDevice->LightEnable( 0, TRUE );
	pDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	pDevice->SetRenderState( D3DRS_AMBIENT, 0x00bfbfbf );
}

void AVDirector::SetFog(float start, float end)
{
	LPDIRECT3DDEVICE9 pDevice = m_Application->GetD3DDevice();
	pDevice->SetRenderState( D3DRS_FOGENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_FOGCOLOR, D3DCOLOR_XRGB(200,200,200) );
	pDevice->SetRenderState( D3DRS_FOGSTART, *(DWORD *)&start );
	pDevice->SetRenderState( D3DRS_FOGEND, *(DWORD *)&end );
	pDevice->SetRenderState( D3DRS_FOGVERTEXMODE,D3DFOG_LINEAR );
}

void AVDirector::SetMatrixProjection(float fovy, float aspect, float zn, float zf)
{
	D3DXMatrixPerspectiveFovLH(&m_matProj, fovy, aspect, zn, zf + PROJECTION_GAP);
	m_Application->GetD3DDevice()->SetTransform( D3DTS_PROJECTION, &m_matProj );

	//컬링용 프로젝션 재설정
	D3DXMatrixPerspectiveFovLH( &m_matProj, fovy, aspect, zn, zf);
}

float AVDirector::GetFPS()
{
	return m_fFPS;
}

void AVDirector::calculateFPS()
{
	static int frameCnt = 0;
	static float elapsed = 0;

	frameCnt++;
	elapsed += m_tickTime;

	if( elapsed >= 1.f )
	{
		m_fFPS = (float)frameCnt / elapsed;
		elapsed = 0;
		frameCnt = 0;
	}
}

void AVDirector::Start(AVScene *scn)
{
	if(m_isStart)
	{
		SAFE_DELETE(m_Scene);
		m_state = AV_STATE_INIT;
	}

	m_isStart = true;
	m_Scene = scn;
}