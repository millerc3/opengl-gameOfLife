#include "SimulationShader.h"

SimulationShader::SimulationShader(const char* vertexPath, const char* fragmentPath) : Shader(vertexPath, fragmentPath) {}

SimulationShader::~SimulationShader() {
    glDeleteBuffers(1, &this->VBO);
    glDeleteBuffers(1, &this->EBO);
    glDeleteVertexArrays(1, &this->VAO);
    glDeleteFramebuffers(1, &this->FBO);
    glDeleteTextures(1, &this->texture);
}

void SimulationShader::Initialize(size_t simWidth, size_t simHeight) {
    this->simWidth = simWidth;
    this->simHeight = simHeight;

    use();
    setIVec2("gridSize", glm::uvec2(simWidth, simHeight));
    setInt("currentState", 0);

    createSimulationQuad();
    createTexture();
}

void SimulationShader::ProvideInitialGrid(std::vector<float>& grid) {
    assert(grid.size() == this->simWidth * this->simHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->simWidth, this->simHeight, GL_RED, GL_FLOAT, grid.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void SimulationShader::RunSimulation() {
    // bind to the simulation FBO
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    glViewport(0, 0, this->simWidth, this->simHeight);

    // setup shader and target texture
    use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture);

    //DebugSimulationTexture();

    // draw
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    DebugSimulationTexture();

    // unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SimulationShader::CopySimulationResultsToTexture(GLuint& targetTexture) {
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 0, 0, this->simWidth, this->simHeight, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SimulationShader::DebugSimulationTexture() {
    std::vector<GLfloat> pixels(this->simWidth * this->simHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, pixels.data());
    assert(1 == 1);
}

void SimulationShader::createSimulationQuad() {
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(this->VAO);

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
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Define the vertex attribute for position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Define the vertex attribute for tetxure coords (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // unbind VAO
    glBindVertexArray(0);
}

void SimulationShader::createTexture() {
    // Create the frame buffer for the simulation
    glGenFramebuffers(1, &this->FBO);

    // Generate the texture the simulation will use
    glGenTextures(1, &this->texture);

    // Configure the texture
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, this->simWidth, this->simHeight, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Attach the texture to the simulation FBO
    glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->texture, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}