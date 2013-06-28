#pragma once

#include <d3d9.h>
#include <stdarg.h>
#include <vector>

typedef struct TEXTURE{

	LPDIRECT3DTEXTURE9	texture;
	wchar_t				path[MAX_PATH];
	int					id;

	TEXTURE()
	{
		texture = NULL;
		ZeroMemory(path, sizeof(wchar_t)*MAX_PATH);
		id = -1;
	}

	~TEXTURE()
	{
		if(texture)
		{
			texture->Release();
			texture = NULL;
		}
	}

}TEXTURE;

class SharedTexture
{
private:
	std::vector<TEXTURE*> m_textureList;
	int m_nSize;

private:
	SharedTexture();
	TEXTURE* CreateTexture(wchar_t *path, int &id);
	
public:
	~SharedTexture(void);

	static SharedTexture* GetSharedTexture();
	void Delete();

public:
	bool addFile(wchar_t *path, int id);
	void deleteAllTexture();

	bool deleteTexture(wchar_t *path);
	bool deleteTexture(int id);

	TEXTURE* GetTexture(wchar_t *path);
	TEXTURE* GetTextureN(wchar_t *name);
	TEXTURE* GetTexture(int id);

	int Size() { return m_nSize; }
};

static SharedTexture *_sharedTexture = NULL;