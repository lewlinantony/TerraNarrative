#include <height_map/height_map.h>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <limits>

HeightMap::HeightMap() : m_width(0), m_height(0) {}

HeightMap::HeightMap(int width, int height) : 
    m_width(width), 
    m_height(height), 
    m_data(width * height, 0.0f) {}


void HeightMap::InitHeightMap(int width, int height, const unsigned char* data) {
    m_width = width;
    m_height = height;
    m_data.resize(width * height);

    for (int i = 0; i < width * height; ++i) {
        m_data[i] = data[i] / 255.0f;
    }
}

float HeightMap::Get(int x, int z) const {
    if (x < 0 || x >= m_width || z < 0 || z >= m_height) {
        throw std::out_of_range("HeightMap: Coordinates out of bounds");
    }
    return m_data[z * m_width + x];
}

void HeightMap::Set(int x, int z, float height) {
    if (x < 0 || x >= m_width || z < 0 || z >= m_height) {
        throw std::out_of_range("HeightMap: Coordinates out of bounds");
    }
    m_data[z * m_width + x] = height;
}

void HeightMap::Print(int precision) const {
    for (int z = 0; z < m_height; ++z) {
        for (int x = 0; x < m_width; ++x) {
            printf("%.*f ", precision, Get(x, z));
        }
        printf("\n");
    }
}

float HeightMap::GetMinHeight() const {
    if (m_data.empty()) return 0.0f;
    return *std::min_element(m_data.begin(), m_data.end());
}

float HeightMap::GetMaxHeight() const {
    if (m_data.empty()) return 0.0f;
    return *std::max_element(m_data.begin(), m_data.end());
}

void HeightMap::Normalize(float minRange, float maxRange) {
    float currentMin = GetMinHeight();
    float currentMax = GetMaxHeight();

    if (std::abs(currentMax - currentMin) < std::numeric_limits<float>::epsilon()) {
        return;
    }

    for (float& height : m_data) {
        height = ((height - currentMin) / (currentMax - currentMin)) * 
                 (maxRange - minRange) + minRange;
    }
}

int HeightMap::GetWidth() const {
    return m_width;
}

int HeightMap::GetHeight() const {
    return m_height;
}

const std::vector<float>& HeightMap::GetData() const {
    return m_data;
}