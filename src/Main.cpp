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
#include <RandomGenerator.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void CreateQuad(GLuint& VAO, GLuint& VBO, GLuint& EBO);
void DeleteQuad(GLuint& VAO, GLuint& currentFBO, GLuint& nextFBO, GLuint& currentTexture, GLuint& nextTexture);
void ApplyGridToTexture(GLuint& FBO, GLuint& texture, std::vector<float>& grid);
void ApplyTextureToGrid(GLuint& texture, std::vector<float>& grid);
void CreatePingPongTextures(GLuint& FBO_A, GLuint& FBO_B, GLuint& textureA, GLuint& textureB);
void PingPongRender(GLuint& VAO, GLuint& currentFBO, GLuint& nextFBO, GLuint& currentTexture, GLuint& nextTexture);

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
    GLuint VAO, VBO, EBO, currentFBO, nextFBO, currentTexture, nextTexture;
    // setup verts/indicies/ebo
    CreateQuad(VAO, VBO, EBO);
    // create FBOs and attach textures to them
    CreatePingPongTextures(currentFBO, nextFBO, currentTexture, nextTexture);

    // Create shader and set the uniforms I need
    Shader shader = Shader();
    shader.use();
    shader.setIVec2("gridSize", glm::uvec2(TEX_WIDTH, TEX_HEIGHT));
    shader.setInt("currentState", 0); // use texture unit 0 as the current state

    // Generate a random grid
    std::vector<float> grid(TEX_WIDTH * TEX_HEIGHT);
    RandomGenerator rng = RandomGenerator();
    rng.fillGridWithNoise(grid);
    ApplyGridToTexture(currentFBO, currentTexture, grid);

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

        // input
        // -----
        processInput(window);

        // when the timer ends, we will redraw
        if (drawTimeRemaining <= 0) {
            std::cout << "Rendering new generation" << std::endl;
            PingPongRender(VAO, currentFBO, nextFBO, currentTexture, nextTexture);
            drawTimeRemaining = .5f;
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    DeleteQuad(VAO, currentFBO, nextFBO, currentTexture, nextTexture);

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

void CreateQuad(GLuint& VAO, GLuint& VBO, GLuint& EBO) {
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

void ApplyGridToTexture(GLuint& FBO, GLuint& texture, std::vector<float>& grid) {
    // Upload grid data to the texture
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEX_WIDTH, TEX_HEIGHT, GL_RED, GL_FLOAT, grid.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ApplyTextureToGrid(GLuint& texture, std::vector<float>& grid) {
    glBindTexture(GL_TEXTURE_2D, texture);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, grid.data());
}


void DeleteQuad(GLuint& VAO, GLuint& currentFBO, GLuint& nextFBO, GLuint& currentTexture, GLuint& nextTexture) {
    glDeleteVertexArrays(1, &VAO);
    glDeleteFramebuffers(1, &currentFBO);
    glDeleteFramebuffers(1, &nextFBO);
    glDeleteTextures(1, &currentTexture);
    glDeleteTextures(1, &nextTexture);
}

void CreatePingPongTextures(GLuint& currentFBO, GLuint& nextFBO, GLuint& currentTexture, GLuint& nextTexture) {
    // Create the two frame buffers
    glGenFramebuffers(1, &currentFBO);
    glGenFramebuffers(1, &nextFBO);

    // generate the two tetures
    glGenTextures(1, &currentTexture);
    glGenTextures(1, &nextTexture);

    GLuint textures[2] = { currentTexture, nextTexture };

    // Create and configure the textures
    for (GLuint& tex : textures) {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, TEX_WIDTH, TEX_HEIGHT, 0, GL_RED, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    // Attach textures to the frame buffers
    glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, currentTexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, nextFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextTexture, 0);

    

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PingPongRender(GLuint& VAO, GLuint& currentFBO, GLuint& nextFBO, GLuint& currentTexture, GLuint& nextTexture) {
    // 1. Bind the nextFBO for rendering (write to the next generation texture)
    glBindFramebuffer(GL_FRAMEBUFFER, nextFBO);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); // Or use the size of the texture

    // 2. Clear the nextFBO (texture) to ensure no leftover data
    glClear(GL_COLOR_BUFFER_BIT);

    // 3. Attach nextTexture to nextFBO (we do this to make sure nextTexture is the target of rendering)
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextTexture, 0);

    // 4. Give the current texture to the shader (this is the input for the shader, previous grid state)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTexture);

    // 5. Draw the quad (which will render the texture and apply Game of Life logic)
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // 6. Unbind the framebuffer when done (0=screen?? I guess)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 7. Swap the current and next textures
    std::swap(currentFBO, nextFBO);
    std::swap(currentTexture, nextTexture);  // Swap textures so that the shader can process the new state in the next frame
}