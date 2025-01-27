#ifndef TERRA_NARRATIVE_RENDERER_H
#define TERRA_NARRATIVE_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <camera/camera.h>
#include <load_shader/shader.h>
#include "terrain/terrain.h"

class Renderer {
private:
    Camera& m_camera;
    Shader& m_shader;
    Terrain& m_terrain;
    float m_aspectRatio;
    float m_near = 0.1f;
    float m_far = 1000.0f;

public:
    Renderer(Camera& camera, Shader& shader, Terrain& terrain, float aspectRatio);
    void render();

private:
    void setupMatrices();
};

#endif