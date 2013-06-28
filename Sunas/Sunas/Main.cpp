#include "MainFrame.h"

INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
	_CrtDumpMemoryLeaks();
	CMainFrame mainFrame(AVRECT(100, 100, 895, 670), hInst);
	mainFrame.Run();
	//	AVDirector::GetDiector()->RunApplication(500.f, hInst, false, AVRECT(100, 100, 800, 600));
	return 0;
}