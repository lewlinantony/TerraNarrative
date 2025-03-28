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
    private:
        int m_iterations;
        float m_minDelta;
        float m_maxDelta;
        
    public:
        enum TerrainType { GENERIC, MOUNTAIN_RANGE, ROLLING_HILLS };
        
    private:
        TerrainType m_terrainType;
        
        // FastNoiseLite wrapper class
        class NoiseGenerator {
        private:
            int m_seed;
        public:
            NoiseGenerator(int seed = 42);
            void setSeed(int seed);
            float getNoise(float x, float y);
            float getOctaveNoise(float x, float y, int octaves, float persistence);
        };
        
        NoiseGenerator m_noise;
        
        float calculateDisplacement(float iteration, float totalIterations);
        std::pair<glm::vec2, glm::vec2> generateFaultPoints(int width, int height, float iteration);
        void createFault(std::vector<std::vector<float>>& heightMap, float iteration);
        void applySimpleErosion(std::vector<std::vector<float>>& heightMap, int iterations);
        void addDetailNoise(std::vector<std::vector<float>>& heightMap, float intensity);
        void smoothTerrain(std::vector<std::vector<float>>& heightMap);

        
    public:
        FaultFormationGenerator(int iterations, float minDelta, float maxDelta);
        void setTerrainType(TerrainType type);
        void generateHeightMap(std::vector<std::vector<float>>& heightMap) override;
    };

class MidpointDisplacementGenerator : public TerrainGenerator {
public:
    MidpointDisplacementGenerator(float roughness = 0.9f, float initialDisplacement = 0.8f);
    void generateHeightMap(std::vector<std::vector<float>>& heightMap) override;

private:
    float m_roughness;
    float m_initialDisplacement;
    int calcNextPowerOfTwo(int size);
    float randomFloatRange(float min, float max);
    void diamondStep(std::vector<std::vector<float>>& heightMap, int size, float displacement);
    void squareStep(std::vector<std::vector<float>>& heightMap, int size, float displacement);
    float getAverageHeight(const std::vector<std::vector<float>>& heightMap, int x, int z, int size);
    
};

class Terrain {
public:

    enum class GenerationType {
        PERLIN_NOISE,
        FAULT_FORMATION,
        MIDPOINT_DISPLACEMENT,
        COUNT
    };

    Terrain();
    Terrain(float yScale, float yShift, int resolution, 
            int width, int height,
            int octaves, float persistence, float frequency,
            int iterations, float minDelta, float maxDelta,
            float roughness, float initialDisplacement);
    ~Terrain();

    void initTexture(Shader& shader, const std::vector<const char*>& texturePaths);
    void generateTerrain(GenerationType type);
    void addedTerrain();
    void render() const;
    float getYScale() const; 
    float getYShift() const; 
    float getheightMin() const;
    float getheightMax() const;    

private:
    // OpenGL buffers
    GLuint m_VAO, m_VBO, m_IBO;
    
    int m_width, m_height;

    float m_heightMin = 0.0f;  
    float m_heightMax = 0.0f;  

    // Perlin Noise
    int m_resolution;
    int m_numStrips;
    int m_numTrisPerStrip;
    int m_octaves;
    float m_persistence,m_frequency;
    float m_yScale;
    float m_yShift;

    //Fault Formation
    int m_iterations;
    float m_minDelta, m_maxDelta;

    //midpoint displacement
    float m_roughness;
    float m_initialDisplacement;    

    // Data storage
    std::vector<float> m_vertexArray;
    std::vector<unsigned int> m_indices;
    std::vector<std::vector<float>> heightMap;
    std::vector<std::vector<float>> currentHeightMap;
    std::vector<std::vector<std::vector<float>>> heightMaps;

    // Terrain generators
    std::unique_ptr<TerrainGenerator> m_currentGenerator;
    

    void addMaps();
    void maxMaps();
    void weightedAddMaps();    

    // Internal methods
    void initializeGLBuffers();
    void generateVertexArray(std::vector<std::vector<float>>& heightMap);
    void generateIndices();
    void setupBuffers();
    void setTerrainGenerator(GenerationType type);
};