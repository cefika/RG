#include "../libs/imgui/include/imgui.h"
#include "../libs/imgui/include/imgui_impl_glfw.h"
#include "../libs/imgui/include/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <rg/Camera.h>
#include <learnopengl/model.h>
#include <rg/Functions.h>

#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadCubemap(vector<std::string> &faces);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 1000;

struct ProgramState {
    float kernel = 0.01f;
    bool ImGuiEnabled = false;
    bool open = false;
    bool effect = false;
    float speed = 0.01f;
    int start = -1;
    Camera camera;
    ProgramState(): camera(glm::vec3(25.0f, 5.0f, 0.0f)) {}
    glm::vec3 elevatorPosition = glm::vec3(-9.8f, -6.0f, 7.6f);
    glm::vec3 doorPosition = glm::vec3(-4.8f, 2.32f, -3.28f);
    glm::mat4 view;

    //void SaveToDisk(std::string path);
    //void LoadFromDisk(std::string path);
};

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
ProgramState* programState;
Function function = Function();
int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    programState = new ProgramState;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    Shader shader(FileSystem::getPath("resources/shaders/vertexShader.vs").c_str(),
                  FileSystem::getPath("resources/shaders/fragmentShader.fs").c_str());
    Shader lightShader(FileSystem::getPath("resources/shaders/lightCube.vs").c_str(),
                       FileSystem::getPath("resources/shaders/lightCube.fs").c_str());
    Shader skyboxShader(FileSystem::getPath("resources/shaders/skybox.vs").c_str(),
                        FileSystem::getPath("resources/shaders/skybox.fs").c_str());
    Shader shaderCubeMaps(FileSystem::getPath("resources/shaders/cubemaps.vs").c_str(),
                          FileSystem::getPath("resources/shaders/cubemaps.fs").c_str());
    Shader screenShader(FileSystem::getPath("resources/shaders/framebuffer.vs").c_str(),
                        FileSystem::getPath("resources/shaders/framebuffer.fs").c_str());
    Shader screenShaderNext(FileSystem::getPath("resources/shaders/framebuffer.vs").c_str(),
                            FileSystem::getPath("resources/shaders/framebufferEffect.fs").c_str());
    Shader lightingShader(FileSystem::getPath("resources/shaders/multi_lights.vs").c_str(),
                          FileSystem::getPath("resources/shaders/multi_lights.fs").c_str());
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,   0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    float floorVertices[] = {
            -7.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 2.0f,
            7.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 2.0f, 2.0f,
            -7.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

            7.0f, 0.0f, -5.0f, 0.0f, -1.0f, 0.0f, 2.0f, 2.0f,
            -7.0f, 0.0f, 5.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            7.0f, 0.0f, 5.0f, 0.0f, -1.0f, 0.0f, 2.0f, 0.0f
    };

    float skyboxVertices[] = {
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
    };
    glm::vec3 pointLightPositions[] = {
            glm::vec3(0.0f, 1.9f, 3.6f),
            glm::vec3(0.4f, 1.9f, 4.0f),
            glm::vec3(0.0f, 1.9f, 4.4f),
            glm::vec3(-0.4f, 1.9f, 4.0f),

            glm::vec3(0.0f, 3.9f, 3.6f),
            glm::vec3(0.4f, 3.9f, 4.0f),
            glm::vec3(0.0f, 3.9f, 4.4f),
            glm::vec3(-0.4f, 3.9f, 4.0f),

            glm::vec3(0.0f, 5.9f, 3.6f),
            glm::vec3(0.4f, 5.9f, 4.0f),
            glm::vec3(0.0f, 5.9f, 4.4f),
            glm::vec3(-0.4f, 5.9f, 4.0f)

    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);


    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), &floorVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    // lightCube VAO
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);

    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int wall = loadTexture(FileSystem::getPath("resources/textures/wall1.jpg").c_str());
    unsigned int floor = loadTexture(FileSystem::getPath("resources/textures/floor.png").c_str());
    unsigned int tile = loadTexture(FileSystem::getPath("resources/textures/stone.jpg").c_str());
    unsigned int stone = loadTexture(FileSystem::getPath("resources/textures/stone.jpg").c_str());
    unsigned int wood = loadTexture(FileSystem::getPath("resources/textures/Wooden_Chair_default.png").c_str());

    vector<std::string> faces {
            FileSystem::getPath("resources/textures/skybox/right.jpg").c_str(),
            FileSystem::getPath("resources/textures/skybox/left.jpg").c_str(),
            FileSystem::getPath("resources/textures/skybox/top.jpg").c_str(),
            FileSystem::getPath("resources/textures/skybox/bottom.jpg").c_str(),
            FileSystem::getPath("resources/textures/skybox/front.jpg").c_str(),
            FileSystem::getPath("resources/textures/skybox/back.jpg").c_str()
    };

    unsigned int cubemapTexture = loadCubemap(faces);

    shader.use();
    shader.setInt("texture1", 0);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    shaderCubeMaps.use();
    shader.setInt("skybox", 0);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);
    // screenShader.setFloat("zoom", programState->zoom);

    screenShaderNext.use();
    screenShaderNext.setInt("screenTexture", 0);

    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    unsigned int textureColorBuffer;
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "DOBARRR!\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    //glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    //glBindVertexArray(0);


    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop

    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);
        // render
        // ------
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // draw our first triangle
        shader.use();

        lightingShader.use();
        lightingShader.setVec3("viewPos", programState->camera.Position);
        lightingShader.setFloat("material.shininess", 32.0f);

        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -1.0f);
        lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

        lightingShader.setVec3("pointLights[0].position",pointLightPositions[0]);
        lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[0].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[0].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[0].constant", 1.0f);
        lightingShader.setFloat("pointLights[0].linear", 0.09);
        lightingShader.setFloat("pointLights[0].quadratic", 0.032);

        lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[1].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[1].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[1].constant", 1.0f);
        lightingShader.setFloat("pointLights[1].linear", 0.09);
        lightingShader.setFloat("pointLights[1].quadratic", 0.032);

        lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        lightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[2].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[2].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[2].constant", 1.0f);
        lightingShader.setFloat("pointLights[2].linear", 0.09);
        lightingShader.setFloat("pointLights[2].quadratic", 0.032);

        lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
        lightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[3].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[3].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[3].constant", 1.0f);
        lightingShader.setFloat("pointLights[3].linear", 0.09);
        lightingShader.setFloat("pointLights[3].quadratic", 0.032);

        lightingShader.setVec3("pointLights[4].position",pointLightPositions[4]);
        lightingShader.setVec3("pointLights[4].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[4].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[4].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[4].constant", 1.0f);
        lightingShader.setFloat("pointLights[4].linear", 0.09);
        lightingShader.setFloat("pointLights[4].quadratic", 0.032);

        lightingShader.setVec3("pointLights[5].position", pointLightPositions[5]);
        lightingShader.setVec3("pointLights[5].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[5].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[5].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[5].constant", 1.0f);
        lightingShader.setFloat("pointLights[5].linear", 0.09);
        lightingShader.setFloat("pointLights[5].quadratic", 0.032);

        lightingShader.setVec3("pointLights[6].position", pointLightPositions[6]);
        lightingShader.setVec3("pointLights[6].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[6].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[6].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[6].constant", 1.0f);
        lightingShader.setFloat("pointLights[6].linear", 0.09);
        lightingShader.setFloat("pointLights[6].quadratic", 0.032);

        lightingShader.setVec3("pointLights[7].position", pointLightPositions[7]);
        lightingShader.setVec3("pointLights[7].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[7].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[7].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[7].constant", 1.0f);
        lightingShader.setFloat("pointLights[7].linear", 0.09);
        lightingShader.setFloat("pointLights[7].quadratic", 0.032);

        lightingShader.setVec3("pointLights[8].position", pointLightPositions[8]);
        lightingShader.setVec3("pointLights[8].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[8].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[8].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[8].constant", 1.0f);
        lightingShader.setFloat("pointLights[8].linear", 0.09);
        lightingShader.setFloat("pointLights[8].quadratic", 0.032);

        lightingShader.setVec3("pointLights[9].position", pointLightPositions[9]);
        lightingShader.setVec3("pointLights[9].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[9].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[9].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[9].constant", 1.0f);
        lightingShader.setFloat("pointLights[9].linear", 0.09);
        lightingShader.setFloat("pointLights[9].quadratic", 0.032);

        lightingShader.setVec3("pointLights[10].position", pointLightPositions[10]);
        lightingShader.setVec3("pointLights[10].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[10].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[10].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[10].constant", 1.0f);
        lightingShader.setFloat("pointLights[10].linear", 0.09);
        lightingShader.setFloat("pointLights[10].quadratic", 0.032);

        lightingShader.setVec3("pointLights[11].position", pointLightPositions[11]);
        lightingShader.setVec3("pointLights[11].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[11].diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("pointLights[11].specular", 0.8f, 0.8f, 0.8f);
        lightingShader.setFloat("pointLights[11].constant", 1.0f);
        lightingShader.setFloat("pointLights[11].linear", 0.09);
        lightingShader.setFloat("pointLights[11].quadratic", 0.032);

        // camera
        programState->view = programState->camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", programState->view);
        lightingShader.setMat4("model", model);

        //floor
        glBindVertexArray(floorVAO);
        function.settingUpFloor(shader, model, floor);
        glBindVertexArray(0);

        // wall
        glBindVertexArray(VAO);
        function.settingUpWall(shader, model, tile, wall, 0.5f);
        function.settingUpPillar(shader, model, stone);
        //function.settingUpWall(shader, model, tile, wall, 8.5f);
        glBindVertexArray(0);

        // tiles
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, wood);
        function.settingUpTilesInPillar(shader, model);
        glBindTexture(GL_TEXTURE_2D, tile);
        function.settingUpTilesInWall(shader, model);
        glBindVertexArray(0);

        // roof
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, tile);
        function.settingUpRoof(shader, model);
        glBindVertexArray(0);

        // light
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", programState->view);
        glBindVertexArray(lightVAO);
        function.settingUpLight(lightShader, model);
        glBindVertexArray(0);

        shaderCubeMaps.use();
        programState->view = programState->camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(programState->camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        shaderCubeMaps.setMat4("view", programState->view);
        shaderCubeMaps.setMat4("projection", projection);
        shaderCubeMaps.setVec3("cameraPos", programState->camera.Position);

        glBindVertexArray(floorVAO);
        glActiveTexture(GL_TEXTURE0);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 3.0f, 0.0f));
        shaderCubeMaps.setMat4("model", model);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(programState->view)));
        skyboxShader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        if (programState->effect) {
            programState->kernel = 0.01f;
            screenShaderNext.use();
        }
        else if (!programState->effect) {
            programState->kernel = programState->kernel + 0.01f >= 1.0f ? 1.0f : programState->kernel + 0.01f;
            screenShader.use();
            screenShader.setFloat("kernel", programState->kernel);
        }

        if (programState->start == 1) {
            programState->view = glm::translate(programState->view, glm::vec3(programState->camera.Position.x,
                                                                              programState->camera.Position.y = programState->camera.Position.y + ((float)glfwGetTime() * deltaTime * programState->speed) - (-9.81 / 2.0f) *  deltaTime * programState->speed *  deltaTime * programState->speed >= 8.32f ?
                                                                                                                8.32f : programState->camera.Position.y + ((float)glfwGetTime() * deltaTime * programState->speed) - (-9.81 / 2.0f) * deltaTime * programState->speed * deltaTime * programState->speed,
                                                                              programState->camera.Position.z));
        }
        else if (programState->start == 0) {
            programState->view = glm::translate(programState->view, glm::vec3(programState->camera.Position.x,
                                                                              programState->camera.Position.y = programState->camera.Position.y - ((float)glfwGetTime() * deltaTime * programState->speed) - (-9.81 / 2.0f) *  deltaTime * programState->speed *  deltaTime * programState->speed <= 2.0f ?
                                                                                                                2.0f : programState->camera.Position.y - ((float)glfwGetTime() * deltaTime * programState->speed) - (-9.81 / 2.0f) * deltaTime * programState->speed * deltaTime * programState->speed,
                                                                              programState->camera.Position.z));
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // disable depth test
        glDisable(GL_DEPTH_TEST);
        // clear all relevant buffers
        // glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        /*glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);*/


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete programState;
    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &floorVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &floorVBO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteBuffers(1, &quadVBO);
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (!programState->ImGuiEnabled) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            programState->camera.ProcessKeyboard(FORWARD, deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            programState->camera.ProcessKeyboard(LEFT, deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            programState->camera.ProcessKeyboard(RIGHT, deltaTime);
        }
    }
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (!programState->ImGuiEnabled) {
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
    }
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
    programState->effect = true;
}
unsigned int loadTexture(char const *path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1) {
            format = GL_RED;
        } else if (nrComponents == 3) {
            format = GL_RGB;
        } else if (nrComponents == 4) {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
unsigned int loadCubemap(vector<std::string> &faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels, n = faces.size();
    for (unsigned int i = 0; i < n; ++i) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                         0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else {
            std::cerr << "Cube map texture face - load failure!\n";

            return -1;
        }
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}