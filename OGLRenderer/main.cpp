#define STB_IMAGE_IMPLEMENTATION

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image/stb_image.h>

#include "shader.hpp"
#include "camera.hpp"
#include "model.hpp"

#include <iostream>
#include <array>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// Camera
Camera camera;
bool uiActive = false;

bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// Material properties
float materialShininess = 32.0f;

// Point Light variables
float ambientStrength = 0.05f;
float diffuseStrength = 0.8f;
float specularStrength = 1.0f;

// Directional light variables
glm::vec3 direction{ -0.2f, -1.0f, -0.3f };
float dirAmbient = 0.05f;
float dirDiffuse = 0.4f;
float dirSpecular = 0.5f;

// Flashlight toggle
bool useFlashlight = true;

// Timing
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

void APIENTRY glDebugOutput(GLenum source,
							GLenum type,
							unsigned int id,
							GLenum severity,
							GLsizei length,
							const char* message,
							const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true); // comment out in release build

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OGLRenderer", NULL, NULL);
	if (window == nullptr) 
	{
		std::cout << "Failed to setup GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Set callback functions
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// enable debug context
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	printf("Max supported Vertex Attribs: %d\n", nrAttributes);

	// configure global state
	glEnable(GL_DEPTH_TEST);

	bool wireframe = false;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	stbi_set_flip_vertically_on_load(true);

	Shader phongShading("shaders/phong.vert", "shaders/phong.frag");
	Shader blinnPhongShading("shaders/blinnPhong.vert", "shaders/blinnPhong.frag");
	Shader gouraudShading("shaders/gouraud.vert", "shaders/gouraud.frag");
	Shader lightSource("shaders/lightSource.vert", "shaders/lightSource.frag");

	Shader modelShader("shaders/model.vert", "shaders/model.frag");
	Model ourModel("assets/models/backpack/backpack.obj");

	std::array <float, 192> cube =
	{
		// positions          // normals		  // tex coords

		// Front face
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,

		// Back face
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

		// Left face
		-0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

		// Right face
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

		 // Bottom face
		 -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
		  0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
		  0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
		 -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,

		 // Top face
		 -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
		  0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
		  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
		 -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f
	};

	std::array<uint32_t, 36> indices = {
		// Front face
		0, 1, 2,  2, 3, 0,
		// Back face
		4, 5, 6,  6, 7, 4,
		// Left face
		8, 9, 10,  10, 11, 8,
		// Right face
		12, 13, 14,  14, 15, 12,
		// Bottom face
		16, 17, 18,  18, 19, 16,
		// Top face
		20, 21, 22,  22, 23, 20
	};

	std::array<glm::vec3, 10> cubePositions =
	{
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	std::array<glm::vec3, 4> pointLightPositions =
	{
		glm::vec3(0.7f, 0.2f, 2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f, 2.0f, -12.0f),
		glm::vec3(0.0f, 0.0f, -3.0f)
	};

	// Light Source VAO configuration
	uint32_t lightVAO, VBO, EBO;
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// IMGUI Initialization
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	enum ShadingMode {PHONG, BLINNPHONG, GOURAUD};
	ShadingMode currentShadingMode = BLINNPHONG;

	bool drawModel = true;
	float modelSize = 1.0f;
	glm::vec3 modelPosition{ 0.0f, 0.0f, 0.0f };
	float modelRotationX = 0.0f;
	float modelRotationY = 0.0f;
	float modelRotationZ = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		processInput(window);

		glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		Shader* activeShader;
		switch(currentShadingMode) 
		{
			case PHONG:
				activeShader = &phongShading;
				break;
			case BLINNPHONG:
				activeShader = &blinnPhongShading;
				break;
			case GOURAUD:
				activeShader = &gouraudShading;
				break;
			default:
				activeShader = &blinnPhongShading;
				break;
		}

		activeShader->use();
		glUniform3fv(glGetUniformLocation(activeShader->ID, "viewPos"), 1, glm::value_ptr(camera.Position));
		// Set Material Properties
		glUniform1f(glGetUniformLocation(activeShader->ID, "material.shininess"), materialShininess);

		// Setting Point Light Properties
		glm::vec3 lightColor(1.0f);
		glm::vec3 diffuseColor = lightColor * glm::vec3(diffuseStrength);
		glm::vec3 ambientColor = lightColor * glm::vec3(ambientStrength);
		glm::vec3 specularColor = glm::vec3(specularStrength);

		// Global attenutation settings
		float constant = 1.0f;
		float linear = 0.09f;
		float quadratic = 0.032f;

		// Directional Light
		glUniform3fv(glGetUniformLocation(activeShader->ID, "dirLight.direction"), 1, glm::value_ptr(direction));
		glUniform3fv(glGetUniformLocation(activeShader->ID, "dirLight.ambient"), 1, glm::value_ptr(glm::vec3(dirAmbient)));
		glUniform3fv(glGetUniformLocation(activeShader->ID, "dirLight.diffuse"), 1, glm::value_ptr(glm::vec3(dirDiffuse)));
		glUniform3fv(glGetUniformLocation(activeShader->ID, "dirLight.specular"), 1, glm::value_ptr(glm::vec3(dirSpecular)));

		// Point Lights
		for (uint32_t i = 0; i < 4; ++i)
		{
			std::string number = std::to_string(i);

			glUniform3fv(glGetUniformLocation(activeShader->ID, ("pointLights[" + number + "].position").c_str()), 1, glm::value_ptr(pointLightPositions[i]));
			glUniform3fv(glGetUniformLocation(activeShader->ID, ("pointLights[" + number + "].ambient").c_str()), 1, glm::value_ptr(ambientColor));
			glUniform3fv(glGetUniformLocation(activeShader->ID, ("pointLights[" + number + "].diffuse").c_str()), 1, glm::value_ptr(diffuseColor));
			glUniform3fv(glGetUniformLocation(activeShader->ID, ("pointLights[" + number + "].specular").c_str()), 1, glm::value_ptr(specularColor));
			glUniform1f(glGetUniformLocation(activeShader->ID, ("pointLights[" + number + "].constant").c_str()), constant);
			glUniform1f(glGetUniformLocation(activeShader->ID, ("pointLights[" + number + "].linear").c_str()), linear);
			glUniform1f(glGetUniformLocation(activeShader->ID, ("pointLights[" + number + "].quadratic").c_str()), quadratic);
		}

		// Spotlights
		if (useFlashlight) {
			glUniform3fv(glGetUniformLocation(activeShader->ID, "spotLight.position"), 1, glm::value_ptr(camera.Position));
			glUniform3fv(glGetUniformLocation(activeShader->ID, "spotLight.direction"), 1, glm::value_ptr(camera.Front));
			glUniform3f(glGetUniformLocation(activeShader->ID, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(activeShader->ID, "spotLight.diffuse"), 1.0f, 1.0f, 1.0f);
			glUniform3f(glGetUniformLocation(activeShader->ID, "spotLight.specular"), 1.0f, 1.0f, 1.0f);
			glUniform1f(glGetUniformLocation(activeShader->ID, "spotLight.constant"), constant);
			glUniform1f(glGetUniformLocation(activeShader->ID, "spotLight.linear"), linear);
			glUniform1f(glGetUniformLocation(activeShader->ID, "spotLight.quadratic"), quadratic);
			glUniform1f(glGetUniformLocation(activeShader->ID, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
			glUniform1f(glGetUniformLocation(activeShader->ID, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));
		}

		// View / Projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(activeShader->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(activeShader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// Render the loaded model
		if (drawModel) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, modelPosition); 
			model = glm::rotate(model, glm::radians(modelRotationX),  glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, glm::radians(modelRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(modelRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::scale(model, glm::vec3(modelSize)); 
			glUniformMatrix4fv(glGetUniformLocation(activeShader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
			ourModel.Draw(modelShader);
		}

		lightSource.use();
		glUniformMatrix4fv(glGetUniformLocation(lightSource.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(lightSource.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));

		glBindVertexArray(lightVAO);
		for (uint32_t i = 0; i < 4; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f));
			glUniformMatrix4fv(glGetUniformLocation(lightSource.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
		}

		ImGui::Begin("OGLRenderer Interface");
		ImGui::Text("Modify Model Properties");
		ImGui::Checkbox("Draw Model", &drawModel);
		ImGui::SliderFloat("Model Scale", &modelSize, 0.5f, 2.0f);
		ImGui::DragFloat3("Model Position", glm::value_ptr(modelPosition), 0.1f);
		ImGui::SliderFloat("Model Rotation X", &modelRotationX, 0.0f, 360.0f);
		ImGui::SliderFloat("Model Rotation Y", &modelRotationY, 0.0f, 360.0f);
		ImGui::SliderFloat("Model Rotation Z", &modelRotationZ, 0.0f, 360.0f);

		ImGui::Separator();
		ImGui::Text("Shading Model");
		bool isPhong = (currentShadingMode == PHONG);
		bool isBlinnPhong = (currentShadingMode == BLINNPHONG);
		bool isGouraud = (currentShadingMode == GOURAUD);

		if (ImGui::RadioButton("Phong", isPhong)) currentShadingMode = PHONG;
		if (ImGui::RadioButton("BlinnPhong", isBlinnPhong)) currentShadingMode = BLINNPHONG;
		if (ImGui::RadioButton("Gouraud", isGouraud)) currentShadingMode = GOURAUD;

		ImGui::Separator();
		ImGui::Text("Material Properties");
		ImGui::SliderFloat("Shininess", &materialShininess, 1.0f, 256.0f);

		ImGui::Separator();
		ImGui::Text("Point Light Properties");
		ImGui::SliderFloat("Ambient Strength", &ambientStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("Diffuse Strength", &diffuseStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("Specular Strength", &specularStrength, 0.0f, 1.0f);

		ImGui::Separator();
		ImGui::Text("Point Light Positions");
		for (uint32_t i = 0; i < pointLightPositions.size(); i++) {
			ImGui::PushID(i);
			std::string label = "Light " + std::to_string(i);
			if (ImGui::CollapsingHeader(label.c_str())) {
				ImGui::DragFloat3("Position", glm::value_ptr(pointLightPositions[i]), 0.1f);
			}
			ImGui::PopID();
		}

		ImGui::Separator();
		ImGui::Text("Directional Light Properties");
		ImGui::DragFloat3("Direction", glm::value_ptr(direction), 0.1f);
		ImGui::SliderFloat("Directional Ambient", &dirAmbient, 0.0f, 1.0f);
		ImGui::SliderFloat("Directional Diffuse", &dirDiffuse, 0.0f, 1.0f);
		ImGui::SliderFloat("Directional Specular", &dirSpecular, 0.0f, 1.0f);

		ImGui::Separator();
		ImGui::Checkbox("Flaslight Toggle", &useFlashlight);
		ImGui::Checkbox("Wireframe Toggle", &wireframe);

		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// De-allocate resources
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) 
	{
		uiActive = !uiActive;
		glfwSetInputMode(window, GLFW_CURSOR, uiActive ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

		firstMouse = true;
	}

	const float cameraSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xPosIn, double yPosIn) 
{
	ImGuiIO& io = ImGui::GetIO();

	if (!io.WantCaptureMouse) {
		float xPos = static_cast<float>(xPosIn);
		float yPos = static_cast<float>(yPosIn);

		if (firstMouse)
		{
			lastX = xPos;
			lastY = yPos;
			firstMouse = false;
		}

		float xOffset = xPos - lastX;
		float yOffset = lastY - yPos; // reversed since y-coordinates go from bottom to top
		lastX = xPos;
		lastY = yPos;

		float sensitivity = 0.1f; // change this value to your liking
		xOffset *= sensitivity;
		yOffset *= sensitivity;

		camera.ProcessMouseMovement(xOffset, yOffset);
	}
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yOffset));
}