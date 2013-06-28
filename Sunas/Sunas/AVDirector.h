#pragma once

#include "AVWinD3D.h"
#include "AVScene.h"
#include <math.h>
#include "Information.h"
#include "Scn_MapTool.h"
#include "AVCamera.h"
#include "toolTypeDef.h"

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#pragma warning (disable:4244)
#pragma warning (disable:4018)
#pragma warning (disable:4927)

enum AVState{
	AV_STATE_INIT,
	AV_STATE_LOOP,
	AV_STATE_END
};

#define SCENE if(m_Scene)m_Scene
#define PROC_SCENE if(scene)scene

class AVDirector
{
private:
	int						m_state;
	float					m_tickTime;

	AVScene					*m_Scene;
	AVWinD3D				*m_Application;

	D3DXMATRIXA16			m_matProj;

	bool					m_bIsFrameCheck;
	float					m_fFPS;
	bool					m_isStart;

private:
	AVDirector(void);
	
public:
	void tickTime();
	void calculateFPS();
	void RunToInnerSystem();

	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void _initMatrix(float fCull);
	void _initLight();


public:
	~AVDirector(void);

	static AVDirector* GetDiector();

	void deleteDirector();
	void RunApplication(float fCullValue, HINSTANCE hIns, HWND parentHandle, bool isChild, AVRECT &windowRect, wchar_t *application = L"Hello World");

	void changeToState(const AVState state);

	AVWinD3D*	GetApplication();
	float		GetTickTime();
	int			GetState();
	AVScene*	GetScene();
	void		SetScene(AVScene* scene);

	float		GetFPS();
	
	//D3DXMATRIXA16* GetMatrixWorld() { return &m_matWorld; }
	//D3DXMATRIXA16* GetMatrixView() { return &m_matView; }
	D3DXMATRIXA16* GetMatrixProjection() { return &m_matProj; }
	void SetFog(float start, float end);

	void SetMatrixProjection(float fovy, float aspect, float zn, float zf);
	bool GetFrameCheck() { return m_bIsFrameCheck; }

	
	bool IsStart()			{ return m_isStart; }
	void Start(AVScene *scn);
};

static AVDirector *_sharedDirector = NULL;