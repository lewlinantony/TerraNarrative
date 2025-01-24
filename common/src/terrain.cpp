#include <terrain/terrain.h>
#include <height_map/height_map.h>
#include <utils/utils.h>
#include <cassert>
#include <stdio.h>

void BaseTerrain::LoadFromFile(const char* pFilename){
    int FileSize = 0;
    unsigned char* p = (unsigned char*)ReadBinaryFile(pFilename, FileSize);

    assert(FileSize);

    m_terrainSize = static_cast<int>(std::sqrt(FileSize / sizeof(float))); 

    m_heightMap.InitHeightMap(m_terrainSize, m_terrainSize, p);  

    m_heightMap.Print();

}
