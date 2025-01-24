#pragma once

#include <vector>
#include <string>
#include <stdexcept>

class HeightMap {
private:
    std::vector<float> m_data;
    int m_width;
    int m_height;

public:
    HeightMap();
    HeightMap(int width, int height);
    void InitHeightMap(int width, int height, const unsigned char* data);
    float Get(int x, int z) const;
    void Set(int x, int z, float height);
    void Print(int precision = 2) const;
    float GetMinHeight() const;
    float GetMaxHeight() const;
    void Normalize(float minRange = 0.0f, float maxRange = 1.0f);
    int GetWidth() const;
    int GetHeight() const;
    const std::vector<float>& GetData() const;
};
