#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <vector>

#include <Shader.h>

class SimulationShader : public Shader
{

public:
	SimulationShader(const char* vertexPath, const char* fragmentPath);

	~SimulationShader();

	void Initialize(size_t simWidth, size_t simHeight);

	void ProvideInitialGrid(std::vector<float>& grid);
	
	void RunSimulation();

	void CopySimulationResultsToTexture(GLuint& targetTexture);

	void DebugSimulationTexture();

private:
	GLuint VAO, VBO, EBO;
	GLuint FBO;
	GLuint texture;

	size_t simWidth = 0, simHeight = 0;

	void createSimulationQuad();

	void createTexture();

};

