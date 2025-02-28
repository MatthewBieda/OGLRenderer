#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.hpp"

#include <iostream>
#include <array>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OGLRenderer", NULL, NULL);
	if (window == nullptr) 
	{
		std::cout << "Failed to setup GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	printf("Max supported Vertex Attribs: %d\n", nrAttributes);

	Shader ourShader("shader.vert", "shader.frag");

	std::array<float, 24> fullscreenQuad = {
		// x, y, z coordinates for two triangles (6 vertices)
		-1.0f, -1.0f, 0.0f, // Bottom-left
		 1.0f, -1.0f, 0.0f, // Bottom-right
		-1.0f,  1.0f, 0.0f, // Top-left

		 1.0f, -1.0f, 0.0f, // Bottom-right
		 1.0f,  1.0f, 0.0f, // Top-right
		-1.0f,  1.0f, 0.0f  // Top-left
	};

	uint32_t VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenQuad), fullscreenQuad.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0); // unbinding VBO
	glBindVertexArray(0); // unbinding VAO

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	uint32_t timeLocation = glGetUniformLocation(ourShader.ID, "iTime");
	uint32_t resolutionLocation = glGetUniformLocation(ourShader.ID, "iResolution");
	uint32_t mouseLocation = glGetUniformLocation(ourShader.ID, "iMouse");

	while (!glfwWindowShouldClose(window)) 
	{	
		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ourShader.use();

		float time = (float)glfwGetTime();
		glUniform1f(timeLocation, time);
		glUniform2f(resolutionLocation, SCR_WIDTH, SCR_HEIGHT);

		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		glUniform2f(mouseLocation, (float)mouseX, (float)mouseY);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	// De-allocate resources
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{
	glViewport(0, 0, width, height);
}