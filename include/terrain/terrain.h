#ifndef TERRA_NARRATIVE_TERRAIN_H
#define TERRA_NARRATIVE_TERRAIN_H

#include <glad/glad.h>
#include <vector>
#include <stdexcept>
#include <stb/stb_image.h>

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

public:
    // Default constructor
    Terrain();
    
    // Parameterized constructor
    Terrain(float yScale, float yShift, int resolution);
    
    // Delete copy constructor and assignment operator
    Terrain(const Terrain&) = delete;
    Terrain& operator=(const Terrain&) = delete;
    
    void loadHeightmap(const char* path);
    void render() const;
    ~Terrain();

private:
    void generateVertices(unsigned char* data);
    void generateIndices();
    void setupBuffers();
    void initializeGLBuffers();
};

#endif