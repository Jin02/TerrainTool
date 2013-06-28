#pragma once

#include <vector>
#include <d3d9.h>
#include <d3dx9.h>

enum TILE_TEXTURE
{
	TILE_TEXTURE_GRASS_1,
	TILE_TEXTURE_GRASS_2,
	TILE_TEXTURE_GRAYSTONE,
	TILE_TEXTURE_SAND_1,
	TILE_TEXTURE_SAND_2,
	TILE_TEXTURE_SAND_3,
	TILE_TEXTURE_TILE,
	TILE_TEXTURE_TILESTONE,
	TILE_TEXTURE_NUM = TILE_TEXTURE_TILESTONE
};

typedef enum TERRAIN_EDIT_TYPE
{
	TERRAIN_EDIT_TYPE_NONE,
	TERRAIN_EDIT_TYPE_BULGING_UP,
	TERRAIN_EDIT_TYPE_BULGING_DOWN,
	TERRAIN_EDIT_TYPE_FLAT_UP,
	TERRAIN_EDIT_TYPE_FLAT_DOWN,
	TERRAIN_EDIT_TYPE_CREATER_UP,
	TERRAIN_EDIT_TYPE_CREATER_DOWN
}TERRAIN_EDIT_TYPE;

typedef enum SPLATTING_OPTION
{
	SPLATTING_OPTION_NONE,
	SPLATTING_OPTION_ADD,
	SPLATTING_OPTION_MINUS
}SPLATTING_OPTION;

typedef struct MapInfo
{
	int nCell;
	int nTile;
	wchar_t *baseImgPath;
	int nScale;
	std::vector<int>				*pvSplattingVisible;
	std::vector<std::pair<int, BYTE*>>	*pvSplatting;
	std::vector<D3DXVECTOR3>		*pvHeightMap;

	MapInfo()
	{
		nCell = 0;
		nTile = 0;
		baseImgPath = NULL;
		nScale = 0;
		pvSplattingVisible = NULL;
		pvSplatting = NULL;
		pvHeightMap = NULL;
	}

}MAPINFO;

#define PROJECTION_GAP 300.f