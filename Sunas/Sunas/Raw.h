#pragma once

#include <string>
#include <vector>
#include <fstream>

class Raw
{
public:
	Raw();
	Raw(std::string fileName, std::vector<unsigned char> *heightMap, int numVertices);
	~Raw(void);

	bool readFile(std::string fileName, std::vector<unsigned char> *heightMap, int numVertices);

};

