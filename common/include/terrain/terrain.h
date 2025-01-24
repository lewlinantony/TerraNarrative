#pragma once

#include <height_map/height_map.h>


class BaseTerrain{
    public:
        BaseTerrain(){}

        void LoadFromFile(const char* pFileName);  

    protected:
        HeightMap m_heightMap;
        int m_terrainSize = 0;



};