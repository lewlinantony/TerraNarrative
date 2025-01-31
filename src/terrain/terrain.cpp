#include <terrain/terrain.h>
#include <stdexcept>
#include <random>
#include <cmath>

// PerlinNoiseGenerator Implementation
PerlinNoiseGenerator::PerlinNoiseGenerator(float frequency, int octaves, float persistence)
    : m_frequency(frequency)
    , m_octaves(octaves)
    , m_persistence(persistence)
    , m_seed(12345)
    , perlin(siv::PerlinNoise(m_seed)) {
}

void PerlinNoiseGenerator::generateHeightMap(std::vector<std::vector<float>>& heightMap) {
    int width = heightMap.size();
    int height = heightMap[0].size();
    
    for(int x = 0; x < width; ++x) {
        for(int z = 0; z < height; ++z) {
            float amplitude = 1.0f;
            float frequency = m_frequency;
            float noiseValue = 0.0f;
            float totalAmplitude = 0.0f;

            for(int i = 0; i < m_octaves; ++i) {
                noiseValue += perlin.noise2D(x * frequency, z * frequency) * amplitude;
                totalAmplitude += amplitude;
                amplitude *= m_persistence;
                frequency *= 2.0f;
            }

            heightMap[x][z] = noiseValue / totalAmplitude;
        }
    }
}

// FaultFormationGenerator Implementation
FaultFormationGenerator::FaultFormationGenerator(int iterations, float minDelta, float maxDelta)
    : m_iterations(iterations)
    , m_minDelta(minDelta)
    , m_maxDelta(maxDelta) {
}

void FaultFormationGenerator::createFault(std::vector<std::vector<float>>& heightMap) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    int width = heightMap.size();
    int height = heightMap[0].size();

    // Generate two random points to create a line
    float x1 = dis(gen) * width;
    float z1 = dis(gen) * height;
    float x2 = dis(gen) * width;
    float z2 = dis(gen) * height;

    // Calculate line parameters
    float a = z2 - z1;
    float b = -(x2 - x1);
    float c = x2 * z1 - x1 * z2;
    float normal = std::sqrt(a * a + b * b);

    // Random height displacement
    float displacement = m_minDelta + dis(gen) * (m_maxDelta - m_minDelta);

    // Apply displacement to all points based on which side of the line they're on
    for(int x = 0; x < width; ++x) {
        for(int z = 0; z < height; ++z) {
            float distance = (a * x + b * z + c) / normal;
            if(distance > 0) {
                heightMap[x][z] += displacement;
            } else {
                heightMap[x][z] -= displacement;
            }
        }
    }
}

void FaultFormationGenerator::generateHeightMap(std::vector<std::vector<float>>& heightMap) {
    // Initialize heightmap to 0
    for(auto& row : heightMap) {
        std::fill(row.begin(), row.end(), 0.0f);
    }

    // Apply fault formation multiple times
    for(int i = 0; i < m_iterations; ++i) {
        createFault(heightMap);
    }

    // Normalize heightmap to [-1, 1] range
    float minHeight = heightMap[0][0];
    float maxHeight = heightMap[0][0];

    // Find min and max heights
    for(const auto& row : heightMap) {
        for(float height : row) {
            minHeight = std::min(minHeight, height);
            maxHeight = std::max(maxHeight, height);
        }
    }

    // Normalize
    float range = maxHeight - minHeight;
    for(auto& row : heightMap) {
        for(float& height : row) {
            height = 2.0f * (height - minHeight) / range - 1.0f;
        }
    }
}

// Terrain Implementation
Terrain::Terrain()
    : Terrain(4.0f, 4.0f, 1, 1000, 1000) {
}

Terrain::Terrain(float yScale, float yShift, int resolution, int width, int height)
    : m_VAO(0)
    , m_VBO(0)
    , m_IBO(0)
    , m_width(width)
    , m_height(height)
    , m_yScale(yScale)
    , m_yShift(yShift)
    , m_resolution(resolution)
    , m_numStrips(0)
    , m_numTrisPerStrip(0)
    , heightMap(width, std::vector<float>(height)) {
    initializeGLBuffers();
}

void Terrain::setTerrainGenerator(GenerationType type) {
    switch(type) {
        case GenerationType::PERLIN_NOISE:
            m_currentGenerator = std::make_unique<PerlinNoiseGenerator>();
            break;
        case GenerationType::FAULT_FORMATION:
            m_currentGenerator = std::make_unique<FaultFormationGenerator>();
            break;
        default:
            throw std::runtime_error("Unknown terrain generation type");
    }
}

void Terrain::generateTerrain(GenerationType type) {
    setTerrainGenerator(type);
    
    if (!m_currentGenerator) {
        throw std::runtime_error("No terrain generator selected");
    }

    m_currentGenerator->generateHeightMap(heightMap);
    
    try {
        generateVertices();
        generateIndices();
        setupBuffers();
    } catch (const std::exception& e) {
        throw;
    }
}

void Terrain::initializeGLBuffers() {
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_IBO);
}

void Terrain::generateVertices() {
    m_vertices.clear();
    m_vertices.reserve(m_width * m_height * 3);

    for(int x = 0; x < m_height; x++) {
        for(int z = 0; z < m_width; z++) {
            float height = heightMap[x][z] * m_yScale - m_yShift;
            
            m_vertices.push_back(-m_height/2.0f + x);
            m_vertices.push_back(height);
            m_vertices.push_back(-m_width/2.0f + z);
        }
    }
}

void Terrain::generateIndices() {
    m_indices.clear();
    m_indices.reserve((m_height - 1) * m_width * 2);

    for(int i = 0; i < m_height-1; i++) {
        for(int j = 0; j < m_width; j++) {
            for(int k = 0; k < 2; k++) {
                m_indices.push_back(j + m_width * (i + k));
            }
        }
    }
    
    m_numStrips = (m_height-1)/m_resolution;
    m_numTrisPerStrip = (m_width/m_resolution)*2-2;
}

void Terrain::setupBuffers() {
    if (m_vertices.empty() || m_indices.empty()) {
        throw std::runtime_error("No vertex or index data to upload to GPU");
    }

    glBindVertexArray(m_VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);    

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
}

void Terrain::render() const {
    glBindVertexArray(m_VAO);
    
    for(unsigned strip = 0; strip < m_numStrips; strip++) {
        glDrawElements(
            GL_TRIANGLE_STRIP,
            m_numTrisPerStrip + 2,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * (m_numTrisPerStrip + 2) * strip)
        );
    }
}

Terrain::~Terrain() {
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_IBO) glDeleteBuffers(1, &m_IBO);
}