#pragma once

#include <glad/glad.h>
#include <vector>
#include <memory>
#include <stb/stb_image.h>
#include <perlin_noise/PerlinNoise.hpp>
#include <load_shader/shader.h>

// Abstract base class for terrain generation algorithms
class TerrainGenerator {
public:
    virtual ~TerrainGenerator() = default;
    virtual void generateHeightMap(std::vector<std::vector<float>>& heightMap) = 0;
};

// Perlin noise terrain generator
class PerlinNoiseGenerator : public TerrainGenerator {
public:
    PerlinNoiseGenerator(float frequency = 0.1f, int octaves = 5, float persistence = 0.5f);
    void generateHeightMap(std::vector<std::vector<float>>& heightMap) override;

private:
    float m_frequency;
    int m_octaves;
    float m_persistence;
    unsigned int m_seed;
    siv::PerlinNoise perlin;
};

// Fault formation terrain generator
class FaultFormationGenerator : public TerrainGenerator {
public:
    FaultFormationGenerator(int iterations = 200, float minDelta = 0.01f, float maxDelta = 0.15f);
    void generateHeightMap(std::vector<std::vector<float>>& heightMap) override;

private:
    int m_iterations;
    float m_minDelta;
    float m_maxDelta;
    void createFault(std::vector<std::vector<float>>& heightMap);
};

class Terrain {
public:
    enum class GenerationType {
        PERLIN_NOISE,
        FAULT_FORMATION
    };

    Terrain();
    Terrain(float yScale, float yShift, int resolution, int width, int height, int octaves, float persistence, float frequency, int iterations, float minDelta, float maxDelta);    ~Terrain();

    void initTexture(Shader& shader, const char* texturePath);
    void generateTerrain(GenerationType type);
    void render() const;

private:
    // OpenGL buffers
    GLuint m_VAO, m_VBO, m_IBO;
    
    int m_width, m_height;

    // Perlin Noise
    float m_yScale, m_yShift;
    int m_resolution;
    int m_numStrips;
    int m_numTrisPerStrip;
    int m_octaves;
    float m_persistence,m_frequency;

    //Fault Formation
    int m_iterations;
    float m_minDelta, m_maxDelta;

    // Data storage
    std::vector<float> m_vertexArray;
    std::vector<unsigned int> m_indices;
    std::vector<std::vector<float>> heightMap;

    // Terrain generators
    std::unique_ptr<TerrainGenerator> m_currentGenerator;
    
    // Internal methods
    void initializeGLBuffers();
    void generateVertexArray();
    void generateIndices();
    void setupBuffers();
    void setTerrainGenerator(GenerationType type);
};