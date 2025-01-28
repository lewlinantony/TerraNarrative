#include <terrain/terrain.h>
#include <stdexcept>
#include <iostream>
#include <perlin_noise/PerlinNoise.hpp>
#include <vector>

Terrain::Terrain()
    : m_VAO(0)
    , m_VBO(0)
    , m_IBO(0)
    , m_width(0)
    , m_height(0)
    , m_channels(0)
    , m_yScale(4.0f)
    , m_yShift(4.0f)
    , m_resolution(1)
    , m_frequency(0.1f)
    , m_noiseValue(0.0f)
    , m_amplitude(1.0f)
    , m_totalAmplitude(0.0f)
    , m_seed(12345)
    , perlin(siv::PerlinNoise(m_seed))    
    , m_numStrips(0)
    , m_numTrisPerStrip(0) {
    initializeGLBuffers();
}

Terrain::Terrain(float yScale, float yShift, int resolution, int width, int height, float frequency, float noiseValue, float amplitude, float totalAmplitude)
    : m_VAO(0)
    , m_VBO(0)
    , m_IBO(0)
    , m_width(width)
    , m_height(height)
    , m_channels(0)
    , m_yScale(yScale)
    , m_yShift(yShift)
    , m_resolution(resolution)
    , m_frequency(frequency)
    , m_noiseValue(noiseValue)
    , m_amplitude(amplitude)
    , m_totalAmplitude(totalAmplitude)    
    , m_seed(12345)
    , perlin(siv::PerlinNoise(m_seed))
    , m_numStrips(0)
    , m_numTrisPerStrip(0) {
    initializeGLBuffers();
}

void Terrain::initializeGLBuffers() {
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_IBO);
}

void Terrain::loadHeightmap() {

    std::vector<std::vector<float>> heightMap(m_width, std::vector<float>(m_height));



    // Generate terrain heights
    for (int x = 0; x < m_width; ++x) {
        for (int z = 0; z < m_height; ++z) {
            // Scale coordinates to adjust noise sampling frequency
            float frequency = 0.1f;
            heightMap[x][z] = perlin.noise2D(x * frequency, z * frequency);
        }
    }
    try {
        generateVertices(heightMap);
        generateIndices();
        setupBuffers();
    } catch (const std::exception& e) {
        throw;
    }

}

void Terrain::generateVertices(std::vector<std::vector<float>> heightMap) {

    m_vertices.clear();
    m_vertices.reserve(m_width * m_height * 3);  // Pre-allocate for better performance
    
    for(unsigned int x = 0; x < m_height; x++) {
        for(unsigned int z = 0; z < m_width; z++) {
            float frequency = m_frequency;           
            float noiseValue = m_noiseValue;
            float amplitude = m_amplitude;
            float totalAmplitude = m_totalAmplitude;

            for (int octave = 0; octave < 5; ++octave) {
                noiseValue += perlin.noise2D(x * frequency, z * frequency) * amplitude;
                totalAmplitude += amplitude;

                frequency *= 2.0f;  // Increase frequency for finer detail
                amplitude *= 0.5f;  // Decrease amplitude
            }

            noiseValue /= totalAmplitude; // Normalize
            heightMap[x][z] = noiseValue;
            float height = noiseValue * m_yScale - m_yShift;

            m_vertices.push_back(-m_height/2.0f + x);
            m_vertices.push_back(height);
            m_vertices.push_back(-m_width/2.0f + z);
        }
    }
}

void Terrain::generateIndices() {
    m_indices.clear();
    m_indices.reserve((m_height - 1) * m_width * 2);  // Pre-allocate for better performance

    for(unsigned int i = 0; i < m_height-1; i++) {
        for(unsigned int j = 0; j < m_width; j++) {
            for(unsigned int k = 0; k < 2; k++) {
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
    glPolygonMode(GL_FRONT_AND_BACK, GL_TRIANGLES);
    
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