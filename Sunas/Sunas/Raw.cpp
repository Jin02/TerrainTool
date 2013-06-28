#include "Raw.h"

Raw::Raw()
{
}

Raw::Raw(std::string fileName, std::vector<unsigned char> *heightMap, int numVertices)
{
	readFile(fileName,heightMap,numVertices);
}

bool Raw::readFile(std::string fileName, std::vector<unsigned char> *heightMap, int numVertices)
{
	std::vector<unsigned char> in(numVertices);

	std::ifstream inFile(fileName.c_str(), std::ios_base::binary);

	if(inFile == 0) return false;

	inFile.read((char*)&in[0], in.size());
	inFile.close();

	heightMap->resize(numVertices);

	for(int i=0; i<in.size(); i++)
		(*heightMap)[i] = in[i];

	return true;
}

Raw::~Raw(void)
{
}
