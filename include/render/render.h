#pragma once

#include <camera/camera.h>
#include <load_shader/shader.h>
#include <terrain/terrain.h>
#include <glm/glm.hpp>

class Renderer {
public:
    Renderer(Camera& camera, Shader& shader, Terrain& terrain, float aspectRatio);
    void render();
    void setTerrainColor(const glm::vec3& color);

private:
    void setupMatrices();

    Camera& m_camera;
    Shader& m_shader;
    Terrain& m_terrain;
    float m_aspectRatio;
    float m_near = 0.1f;
    float m_far = 1000.0f;
    glm::vec3 m_terrainColor = glm::vec3(0.0157f, 0.5294f, 0.1176f);
};