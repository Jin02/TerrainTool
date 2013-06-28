#include "SharedTexture.h"
#include "AVDirector.h"

SharedTexture::SharedTexture(void)
{
	m_nSize = 0;
}

SharedTexture::~SharedTexture(void)
{
	deleteAllTexture();
}

SharedTexture* SharedTexture::GetSharedTexture()
{
	if( _sharedTexture == NULL ) 
		_sharedTexture = new SharedTexture;
	return _sharedTexture;
}

void SharedTexture::Delete()
{
	if(_sharedTexture) 
		delete _sharedTexture;
	_sharedTexture = NULL;
}

bool SharedTexture::addFile(wchar_t *path, int id)
{
	std::vector<TEXTURE*>::iterator iter;

	for(iter = m_textureList.begin() ; iter != m_textureList.end() ; iter++)
		if( (*iter)->id == id || ( wcscmp((*iter)->path, path) == 0) )		return false;

	TEXTURE *tex = CreateTexture(path, id);
	
	if(tex)
	{
		m_textureList.push_back(tex);
		m_nSize = m_textureList.size();
	}
	else
		return false;

	return true;
}

TEXTURE* SharedTexture::CreateTexture(wchar_t *path, int &id)
{
	LPDIRECT3DTEXTURE9 tex = NULL;
	
	if( FAILED(D3DXCreateTextureFromFile(AVDirector::GetDiector()->GetApplication()->GetD3DDevice(),
		path,&tex)) )
	{
		return NULL;
	}

	TEXTURE *texture = new TEXTURE;
	texture->id		 = id;
	texture->texture = tex;

	wcscpy(texture->path, path);

	return texture;
}

void SharedTexture::deleteAllTexture()
{
	for(int i = 0; i < m_textureList.size(); i++)
		SAFE_DELETE(m_textureList[i]);
	m_textureList.clear();

	m_nSize = 0;
}

TEXTURE* SharedTexture::GetTexture(wchar_t *path)
{
	std::vector<TEXTURE*>::iterator iter;

	for(iter = m_textureList.begin() ; iter != m_textureList.end() ; iter++)
	{
		if( wcscmp((*iter)->path, path) == 0 )
		{
			return (*iter);
		}
	}

	return NULL;
}

TEXTURE* SharedTexture::GetTexture(int id)
{
	std::vector<TEXTURE*>::iterator iter;

	for(iter = m_textureList.begin() ; iter != m_textureList.end() ; iter++)
	{
		if( (*iter)->id == id )
		{
			return (*iter);
		}
	}

	return NULL;
}

TEXTURE* SharedTexture::GetTextureN(wchar_t *name)
{
	std::vector<TEXTURE*>::iterator iter;

	for(iter = m_textureList.begin() ; iter != m_textureList.end() ; iter++)
	{
		if(wcsstr((*iter)->path, name) != NULL)
		{
			return (*iter);
		}
	}

	return NULL;
}

bool SharedTexture::deleteTexture(wchar_t *path)
{
	for(int i = 0; i < m_textureList.size(); i++ )
	{
		if(wcscmp(m_textureList[i]->path, path) == 0)
		{
			SAFE_DELETE(m_textureList[i]);
			m_textureList.erase(m_textureList.begin() + i);
			m_nSize = m_textureList.size();
			return true;
		}
	}

	return false;
}

bool SharedTexture::deleteTexture(int id)
{
	for(int i = 0; i < m_textureList.size(); i++ )
	{
		if( m_textureList[i]->id == id )
		{
			SAFE_DELETE(m_textureList[i]);
			m_textureList.erase(m_textureList.begin() + i);
			return true;
		}
	}

	return false;
}