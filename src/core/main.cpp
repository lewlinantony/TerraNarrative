#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <load_shader/shader.h> 
#include <camera/camera.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <terrain/terrain.h>
#include <render/render.h>

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
float ASPECT_RATIO = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;


class TerraNarrative{
   
   public:
        TerraNarrative(){}

        void init(){
            initGLFW();
            createWindow();
            setupGLFWCallbacks();
            initGLAD();
            initImGui();  
            initCamera();
            initShaders();
            initTerrain();
            initRenderer();
        }


        void processInput(GLFWwindow* window) {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, true);
            }

            static bool m_lastFState = false;
            bool m_currentFState = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
            if (m_currentFState && !m_lastFState) {
                m_fullscreen = !m_fullscreen;
                if (m_fullscreen) {
                    glfwGetWindowPos(window, &m_windowed_x, &m_windowed_y);
                    glfwGetWindowSize(window, &m_windowed_width, &m_windowed_height);
                    GLFWmonitor* m_monitor = glfwGetPrimaryMonitor();
                    const GLFWvidmode* m_mode = glfwGetVideoMode(m_monitor);
                    glfwSetWindowMonitor(window, m_monitor, 0, 0, m_mode->width, m_mode->height, m_mode->refreshRate);
                } else {
                    glfwSetWindowMonitor(window, nullptr, m_windowed_x, m_windowed_y, 
                                    m_windowed_width, m_windowed_height, 0);
                }
            }
            m_lastFState = m_currentFState;

            bool currentTabState = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
            if (currentTabState && !m_lastTabState) {
                m_cursorEnabled = !m_cursorEnabled;
                glfwSetInputMode(window, GLFW_CURSOR, 
                    m_cursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
                m_firstMouse = true;
                
                // Toggle ImGui input capture based on cursor state
                ImGui::GetIO().ConfigFlags = m_cursorEnabled ? 0 : ImGuiConfigFlags_NoMouse | ImGuiConfigFlags_NoKeyboard;
            }
            m_lastTabState = currentTabState;
            
            // Process WASD input when cursor is disabled
            if (!m_cursorEnabled) {
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                    camera.ProcessKeyboard(FORWARD, m_deltaTime);
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                    camera.ProcessKeyboard(BACKWARD, m_deltaTime);
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                    camera.ProcessKeyboard(LEFT, m_deltaTime);
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                    camera.ProcessKeyboard(RIGHT, m_deltaTime);
            }
        }

        void renderImGuiControls() {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Terrain Controls");
            
            // Terrain parameters
            bool paramsChanged = false;
            paramsChanged |= ImGui::SliderFloat("Height Scale", &m_yScale, 0.0f, 1.0f, "%.3f");
            paramsChanged |= ImGui::SliderFloat("Height Shift", &m_yShift, 0.0f, 32.0f, "%.1f");
            paramsChanged |= ImGui::SliderInt("Resolution", &m_resolution, 1, 10);
            
            if (paramsChanged) {
                // Recreate terrain with new parameters
                delete m_terrain;
                initTerrain();
                delete m_renderer;
                initRenderer();
            }

            // Camera info
            if (ImGui::CollapsingHeader("Camera Info")) {
                glm::vec3 pos = camera.Position;
                ImGui::Text("Position: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
                ImGui::Text("FOV: %.1f", camera.Zoom);
            }

            // Rendering options
            if (ImGui::CollapsingHeader("Render Settings")) {
                ImGui::Checkbox("Wireframe Mode", &m_isWireframe);
                if (m_isWireframe) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                } else {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
            }

            // Performance metrics
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                1000.0f / ImGui::GetIO().Framerate, 
                ImGui::GetIO().Framerate);

            ImGui::End();
        }

        void run() {
            while(!glfwWindowShouldClose(window)) {
                float currentFrame = static_cast<float>(glfwGetTime());
                m_deltaTime = currentFrame - m_lastFrame;
                m_lastFrame = currentFrame;

                processInput(window);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                m_renderer->render();
                
                // Always render ImGui
                renderImGuiControls();
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                glfwSwapBuffers(window);
                glfwPollEvents();
            }
            
            // Cleanup
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            
            glfwDestroyWindow(window);
            glfwTerminate();                
        }

    private:

        GLFWwindow* window = NULL;
        Camera camera;
        Shader shader;
        Terrain* m_terrain = nullptr;
        Renderer* m_renderer = nullptr;        

        bool m_cursorEnabled = false;
        bool m_lastTabState = false;  
        bool m_firstMouse = true;
        bool m_fullscreen = false;
        int m_windowed_width = 800;  // default window width
        int m_windowed_height = 600; // default window height
        int m_windowed_x = 0;
        int m_windowed_y = 0;
        bool m_lastFState = false;

        float m_deltaTime = 0.0f;
        float m_lastFrame = 0.0f; // Time of last frame



        bool m_isWireframe = false; 
        double m_lastX = WINDOW_WIDTH / 2.0;
        double m_lastY = WINDOW_HEIGHT / 2.0;
        int m_widthImg;
        int m_heightImg;
        int m_nrChannels;

        int m_numStrips;
        int m_numTrisPerStrip;

        float m_yScale = 64.0f / 256.0f;
        float m_yShift = 16.0f;  
        int m_resolution = 1;


        float m_near  =  0.1f;
        float m_far   =  1000.0f;        

        const char* m_vertexShader = "../assets/shaders/terrain.vert";
        const char* m_fragShader = "../assets/shaders/terrain.frag";     

        const char* m_heightMapPath = "../assets/data/iceland_heightmap.png";
        
        GLuint m_VAO,m_VBO,m_IBO;

        glm::vec3 m_cameraPos   = glm::vec3(0.0f, 40.0f,  3.0f);
        glm::vec3 m_cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);        
        float m_yaw   = -90.0f;	
        float m_pitch =  0.0f;

        std::vector<float> m_vertices;
        std::vector<unsigned int> m_indices;


        void initGLFW(){
            if(!glfwInit()){
                std::cout<<"noo glfww"<<std::endl;

            }
            else{
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
                glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);        
                std::cout<<"yes glfw"<<std::endl;
            }     
        }

        void createWindow(){
            window = glfwCreateWindow(WINDOW_WIDTH,WINDOW_HEIGHT,"texture",NULL,NULL);
            if(!window){
                std::cout<<"noo window"<<std::endl;
            }
            else{
                std::cout<<"yes window"<<std::endl;
            }

            glfwMakeContextCurrent(window);
            glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
        }

        void initGLAD(){
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
                std::cerr << "noo glad" << std::endl;
            }   
            else{
                std::cout<<"yes glad"<<std::endl;
            }
        }
        
        void initTerrain(){
            try {
                m_terrain = new Terrain(m_yScale, m_yShift, m_resolution);
                m_terrain->loadHeightmap(m_heightMapPath);
            } catch (const std::runtime_error& e) {
                std::cerr << "Failed to load terrain: " << e.what() << std::endl;
                throw;
            }                
        }

        void initShaders(){
            shader = Shader(m_vertexShader,m_fragShader);                        
        }

        void initCamera(){
            camera = Camera(m_cameraPos, m_cameraUp,  m_yaw, m_pitch);
        }

        void initImGui() {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            
            // Setup ImGui style
            ImGui::StyleColorsDark();
            
            // Setup Platform/Renderer backends
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init("#version 330");
        }

        void initRenderer(){
            m_renderer = new Renderer(camera, shader, *m_terrain, ASPECT_RATIO);
        }

        void setupGLFWCallbacks() {

            glfwMakeContextCurrent(window);
            glfwGetFramebufferSize(window,&WINDOW_WIDTH,&WINDOW_HEIGHT);            
            // Static wrapper functions that retrieve the instance

            glfwSetWindowUserPointer(window, this);

            glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
                glViewport(0, 0, width, height);
            });

            glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
                TerraNarrative* app = static_cast<TerraNarrative*>(glfwGetWindowUserPointer(w));
                if (app) app->mouseCallback(w, button, action, mods);
            });

            glfwSetCursorPosCallback(window, [](GLFWwindow* w, double xpos, double ypos) {
                TerraNarrative* app = static_cast<TerraNarrative*>(glfwGetWindowUserPointer(w));
                if (app) app->cursorPosCallback(w, xpos, ypos);
            });

            glfwSetScrollCallback(window, [](GLFWwindow* w, double xoffset, double yoffset) {
                TerraNarrative* app = static_cast<TerraNarrative*>(glfwGetWindowUserPointer(w));
                if (app) app->scrollCallback(w, xoffset, yoffset);
            });
        }

        void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
            // Implement mouse button logic if needed
        }        
    
        void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
            if (!m_cursorEnabled && !ImGui::GetIO().WantCaptureMouse) {
                if (m_firstMouse) {
                    m_lastX = xpos;
                    m_lastY = ypos;
                    m_firstMouse = false;
                }

                float xoffset = xpos - m_lastX;
                float yoffset = m_lastY - ypos;

                m_lastX = xpos;
                m_lastY = ypos;

                camera.ProcessMouseMovement(xoffset, yoffset, true);
            }
        }

        void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
            camera.ProcessMouseScroll(static_cast<float>(yoffset));
        }                
};

int main(){

    TerraNarrative app = TerraNarrative();

    app.init();    
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    app.run();    

    return 0;
}