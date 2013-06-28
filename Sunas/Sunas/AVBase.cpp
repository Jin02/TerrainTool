#include "AVBase.h"


AVBase::AVBase(void)
{
}


AVBase::~AVBase(void)
{
}

bool AVBase::Update()
{
	//NULL
	return true;
}

bool AVBase::Render()
{
	//NULL
	return true;
}

void AVBase::addObject(AVBase* object)
{
	m_vector.push_back(object);
}

void AVBase::deleteObject(AVBase* object)
{
//	std::vector<AVBase*>::iterator iter;

	for(int i = 0; i < m_vector.size(); i++)
	{
		if( m_vector[i] == object )
			m_vector.erase(m_vector.begin()+i);
	}
}

void AVBase::deleteAllObject()
{
	m_vector.clear();
}

void AVBase::InnerUpdate()
{
	std::vector<AVBase*>::iterator iter;

	for(iter = m_vector.begin(); iter != m_vector.end(); iter++)
	{
		(*iter)->Update();
		(*iter)->Render();
	}
}