#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <load_shader/shader.h> 
#include <camera/camera.h>
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
            initCamera();
            initShaders();
            initTerrain();
        }

        void processInput(GLFWwindow* window){
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);

            // Check TAB state
            bool currentTabState = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
            if (currentTabState && !m_lastTabState) {  // Only trigger on press, not hold
                m_cursorEnabled = !m_cursorEnabled;
                glfwSetInputMode(window, GLFW_CURSOR, 
                    m_cursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
                // Reset firstMouse when switching modes to prevent camera jump
                m_firstMouse = true;
            }
            m_lastTabState = currentTabState;
            
            if (!m_cursorEnabled) {
                float cameraSpeed = 2.5f * m_deltaTime;
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

        void run(){
            while(!glfwWindowShouldClose(window)){
                float currentFrame = static_cast<float>(glfwGetTime());
                m_deltaTime = currentFrame - m_lastFrame;
                m_lastFrame = currentFrame;

                processInput(window);

                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                shader.use();

                glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)ASPECT_RATIO, m_near, m_far);
                shader.setMat4("projection", projection);

                // camera/view transformation
                glm::mat4 view = camera.GetViewMatrix();   
                shader.setMat4("view", view);

                glm::mat4 model = glm::mat4(1.0f);
                shader.setMat4("model", model);

                glBindVertexArray(m_VAO);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                
                for(unsigned strip = 0; strip < m_numStrips; strip++)
                {
                    glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
                                m_numTrisPerStrip+2,   // number of m_indices to render
                                GL_UNSIGNED_INT,     // index data type
                                (void*)(sizeof(unsigned) * (m_numTrisPerStrip+2) * strip)); // offset to starting index
                }        

                glfwSwapBuffers(window);
                glfwPollEvents();
            }
            glfwDestroyWindow(window);
            glfwTerminate();                
        }

    private:

        GLFWwindow* window = NULL;
        Camera camera;
        Shader shader;

        bool m_cursorEnabled = false;
        bool m_lastTabState = false;  
        bool m_firstMouse = true;
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


        float m_deltaTime = 0.0f;	// Time between current frame and last frame
        float m_lastFrame = 0.0f; // Time of last frame

        float m_near  =  0.1f;
        float m_far   =  1000.0f;        

        const char* m_vertexShader = "../assets/shaders/terrain.vert";
        const char* m_fragShader = "../assets/shaders/terrain.frag";        
        
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

            unsigned char *data = stbi_load("../assets/data/iceland_heightmap.png", &m_widthImg, &m_heightImg, &m_nrChannels, 0);
            if (data)
            {
                std::cout << "Loaded heightmap of size " << m_heightImg << " x " << m_widthImg << std::endl;
            }
            else
            {
                std::cout << "Failed to load texture" << std::endl;
            }



            for(unsigned int i = 0; i < m_heightImg; i++)
            {
                for(unsigned int j = 0; j < m_widthImg; j++)
                {
                    // retrieve texel for (i,j) tex coord
                    unsigned char* texel = data + (j + m_widthImg * i) * m_nrChannels;
                    // raw eightImg at coordinate
                    unsigned char y = texel[0];

                    // vertex
                    m_vertices.push_back( -m_heightImg/2.0f + i ); //scaling the x and z to -128 to 129 from 0 to 256 by adding -128
                    m_vertices.push_back( (int)y * m_yScale - m_yShift); // v.y
                    m_vertices.push_back( -m_widthImg/2.0f + j );        // v.z
                }
            }  
            stbi_image_free(data);

            for(unsigned int i = 0; i < m_heightImg-1; i++)       // for each row a.k.a. each strip
            {
                for(unsigned int j = 0; j < m_widthImg; j++)      // for each column
                {
                    for(unsigned int k = 0; k < 2; k++)      // for each side of the strip
                    {
                        m_indices.push_back(j + m_widthImg * (i + k));
                    }
                }
            }    

            m_numStrips = (m_heightImg-1)/m_resolution;
            m_numTrisPerStrip = (m_widthImg/m_resolution)*2-2;

            // PUTTIUNG THE m_vertices AND SHI INTO THE BUFFER

            glGenVertexArrays(1,&m_VAO);
            glBindVertexArray(m_VAO);
            
            glGenBuffers(1,&m_VBO);
            glBindBuffer(GL_ARRAY_BUFFER,m_VBO);
            glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), &m_vertices[0], GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            glEnableVertexAttribArray(0);    


            glGenBuffers(1,&m_IBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);
        }

        void initShaders(){
            shader = Shader(m_vertexShader,m_fragShader);                        
        }

        void initCamera(){
            camera = Camera(m_cameraPos, m_cameraUp,  m_yaw, m_pitch);
        }

        void renderScene(){
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
            if (!m_cursorEnabled) {
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