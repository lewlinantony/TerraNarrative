#pragma once
#include <iostream>
#include <camera/camera.h>
#include <load_shader/shader.h>
#include <terrain/terrain.h>
#include <glm/glm.hpp>

class Renderer {
public:
    Renderer(Camera& camera, Shader& shader, Terrain& terrain, float aspectRatio);
    void render();

private:
    void setupMatrices();

    Camera& m_camera;
    Shader& m_shader;
    Terrain& m_terrain;
    float m_aspectRatio;
    float m_near = 0.1f;
    float m_far = 1000.0f;
};