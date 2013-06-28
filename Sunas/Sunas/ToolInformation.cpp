#include "ToolInformation.h"


CToolInformation::CToolInformation(void)
{
}

CToolInformation::~CToolInformation(void)
{
}

CToolInformation* CToolInformation::GetToolInfo()
{
	if( _toolInfo == NULL )
		_toolInfo = new CToolInformation;
	return _toolInfo;
}

void CToolInformation::Delete()
{
	if( _toolInfo )
		delete _toolInfo;
	_toolInfo = NULL;
}