#include <terrain/terrain.h>
#include <stdexcept>
#include <iostream>

Terrain::Terrain()
    : m_VAO(0)
    , m_VBO(0)
    , m_IBO(0)
    , m_width(0)
    , m_height(0)
    , m_channels(0)
    , m_yScale(64.0f / 256.0f)
    , m_yShift(16.0f)
    , m_resolution(1)
    , m_numStrips(0)
    , m_numTrisPerStrip(0) {
    initializeGLBuffers();
}

Terrain::Terrain(float yScale, float yShift, int resolution)
    : m_VAO(0)
    , m_VBO(0)
    , m_IBO(0)
    , m_width(0)
    , m_height(0)
    , m_channels(0)
    , m_yScale(yScale)
    , m_yShift(yShift)
    , m_resolution(resolution)
    , m_numStrips(0)
    , m_numTrisPerStrip(0) {
    initializeGLBuffers();
}

void Terrain::initializeGLBuffers() {
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_IBO);
}

void Terrain::loadHeightmap(const char* path) {
    if (!path) {
        throw std::runtime_error("Invalid heightmap path: null pointer");
    }

    std::cout << "Loading heightmap from: " << path << std::endl;
    
    unsigned char* data = stbi_load(path, &m_width, &m_height, &m_channels, 0);
    if (!data) {
        const char* error = stbi_failure_reason();
        std::string errorMsg = "Failed to load heightmap: ";
        errorMsg += (error ? error : "unknown error");
        throw std::runtime_error(errorMsg);
    }

    std::cout << "Successfully loaded image: " << m_width << "x" << m_height 
              << " with " << m_channels << " channels" << std::endl;
    
    try {
        generateVertices(data);
        generateIndices();
        setupBuffers();
    } catch (const std::exception& e) {
        stbi_image_free(data);
        throw;
    }
    
    stbi_image_free(data);
}

void Terrain::generateVertices(unsigned char* data) {
    if (!data || m_width <= 0 || m_height <= 0) {
        throw std::runtime_error("Invalid heightmap data or dimensions");
    }

    m_vertices.clear();
    m_vertices.reserve(m_width * m_height * 3);  // Pre-allocate for better performance
    
    for(unsigned int i = 0; i < m_height; i++) {
        for(unsigned int j = 0; j < m_width; j++) {
            unsigned char* texel = data + (j + m_width * i) * m_channels;
            unsigned char y = texel[0];

            m_vertices.push_back(-m_height/2.0f + i);
            m_vertices.push_back((int)y * m_yScale - m_yShift);
            m_vertices.push_back(-m_width/2.0f + j);
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