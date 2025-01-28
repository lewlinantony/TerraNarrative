#ifndef TERRA_NARRATIVE_TERRAIN_H
#define TERRA_NARRATIVE_TERRAIN_H

#include <glad/glad.h>
#include <vector>
#include <stdexcept>
#include <stb/stb_image.h>
#include <perlin_noise/PerlinNoise.hpp>
#include <vector>


class Terrain {
private:
    GLuint m_VAO, m_VBO, m_IBO;
    std::vector<float> m_vertices;
    std::vector<unsigned int> m_indices;
    int m_width, m_height, m_channels;
    float m_yScale;
    float m_yShift;
    int m_resolution;
    int m_numStrips;
    int m_numTrisPerStrip;
    float m_frequency;
    float m_noiseValue;
    float m_amplitude;
    float m_totalAmplitude;
    unsigned int m_seed; 
    siv::PerlinNoise perlin; // Seed


public:
    // Default constructor
    Terrain();
    
    // Parameterized constructor
    Terrain(float yScale, float yShift, int resolution, int width, int height, float frequency=0.1f, float noiseValue=0.0f, float amplitude=1.0f, float totalAmplitude=0.0f);
    
    // Delete copy constructor and assignment operator
    Terrain(const Terrain&) = delete;
    Terrain& operator=(const Terrain&) = delete;
    
    void loadHeightmap();
    void render() const;
    ~Terrain();

private:
    void generateVertices(std::vector<std::vector<float>> heightMap);
    void generateIndices();
    void setupBuffers();
    void initializeGLBuffers();
};

#endif