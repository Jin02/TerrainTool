#pragma once

#include "Information.h"
#include "toolTypeDef.h"
#include <vector>

class CToolInformation
{
public:

private:
	CToolInformation(void);

public:
	~CToolInformation(void);

	static CToolInformation* GetToolInfo();
	void Delete();
};

static CToolInformation *_toolInfo = NULL;