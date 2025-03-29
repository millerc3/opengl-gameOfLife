#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Shader.h>
#include <SimulationShader.h>
#include <RandomGenerator.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void CreateRenderQuad(GLuint& VAO, GLuint& VBO, GLuint& EBO);
void DeleteRenderQuad(GLuint& VAO, GLuint& VBO, GLuint& EBO, GLuint& renderTexture);

// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;

const unsigned int TEX_WIDTH = 256;
const unsigned int TEX_HEIGHT = 256;

// time
float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

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
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Create GL objects
    GLuint VAO, VBO, EBO, renderTexture;
    // setup verts/indicies/ebo
    CreateRenderQuad(VAO, VBO, EBO);

    Shader renderShader = Shader();
    renderShader.use();
    renderShader.setIVec2("gridSize", glm::uvec2(TEX_WIDTH, TEX_HEIGHT));
    renderShader.setInt("currentState", 0); // use texture unit 0 as the current state

    // Setup simulation shader program
    SimulationShader simulationShader("src/shaders/shader.vert", "src/shaders/simulation.frag");
    simulationShader.Initialize(TEX_WIDTH, TEX_HEIGHT);

    // Create render texture
    glGenTextures(1, &renderTexture);
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, TEX_WIDTH, TEX_HEIGHT, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Generate a random grid
    std::vector<float> grid(TEX_WIDTH * TEX_HEIGHT);
    RandomGenerator rng = RandomGenerator();
    rng.fillGridWithNoise(grid);
    // New apply to grid
    simulationShader.ProvideInitialGrid(grid);


    // set timer for re-rendering to 0 to immediately render 
    float drawTimeRemaining = 0;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // handle time
        float currentFrameTime = glfwGetTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        drawTimeRemaining -= deltaTime;

        printf("FPS: %d\r", int(1.0f / deltaTime));

        // input
        // -----
        processInput(window);

        if (drawTimeRemaining <= 0) {
            //std::cout << "Rendering new generation" << std::endl;

            simulationShader.RunSimulation();
            //simulationShader.DebugSimulationTexture();
            simulationShader.CopySimulationResultsToTexture(renderTexture);
            drawTimeRemaining = .1f;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT);
        renderShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderTexture);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    DeleteRenderQuad(VAO, VBO, EBO, renderTexture);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void CreateRenderQuad(GLuint& VAO, GLuint& VBO, GLuint& EBO) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    const float vertices[] = {
        // Positions      // Texture Coordinates
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,   // Top-right
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,   // Bottom-right
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,   // Bottom-left
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f    // Top-left
    };

    const unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Define the vertex attribute for position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Define the vertex attribute for texture coordinates (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);  // Unbind VAO
}


void DeleteRenderQuad(GLuint& VAO, GLuint& VBO, GLuint& EBO, GLuint& renderTexture) {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteTextures(1, &renderTexture);
}
