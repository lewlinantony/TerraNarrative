#include <render/render.h>
#include <camera/camera.h>
#include <load_shader/shader.h>

Renderer::Renderer(Camera& camera, Shader& shader, Terrain& terrain, float aspectRatio)
    : m_camera(camera), m_shader(shader), m_terrain(terrain), m_aspectRatio(aspectRatio) {}

void Renderer::render() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader.use();
    setupMatrices();

    m_terrain.render();
}

void Renderer::setupMatrices() {
    glm::mat4 projection = glm::perspective(glm::radians(m_camera.Zoom), m_aspectRatio, m_near, m_far);
    m_shader.setMat4("projection", projection);

    glm::mat4 view = m_camera.GetViewMatrix();   
    m_shader.setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    m_shader.setMat4("model", model);
}