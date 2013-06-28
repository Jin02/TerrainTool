#pragma once

#include <stdarg.h>
#include <vector>

class AVBase
{
private:
	std::vector<AVBase*> m_vector;

public:
	AVBase(void);
	virtual ~AVBase(void);

	virtual bool Update();
	virtual bool Render();

protected:
	void addObject(AVBase* object);
	void deleteObject(AVBase* object);
	void deleteAllObject();

public:
	void InnerUpdate();
};