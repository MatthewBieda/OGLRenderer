#define STB_IMAGE_IMPLEMENTATION

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"

#include "shader.hpp"
#include "camera.hpp"
#include "model.hpp"

#include <iostream>
#include <array>
#include <filesystem>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void processInput(GLFWwindow* window);
void processControllerInput();
void renderQuad();
void LoadModelFolders();

unsigned int loadCubemap(std::vector<std::string> faces);

// Screen 
const int SCR_WIDTH = 1920;
const int SCR_HEIGHT = 1080;

// Camera
Camera camera;
bool uiActive = false;

bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// Gamepad State
bool controllerConnected = false;
float leftStickX = 0.0f;
float leftStickY = 0.0f;

float rightStickX = 0.0f;
float rightStickY = 0.0f;

float leftTrigger = 0.0f;
float rightTrigger = 0.0f;

bool aButtonPressed = false;
bool bButtonPressed = false;
bool xButtonPressed = false;
bool yButtonPressed = false;

bool leftBumper = false;
bool rightBumper = false;

bool dpadUp = false;
bool dpadDown = false;
bool dpadLeft = false;
bool dpadRight = false;

const float CONTROLLER_DEADZONE = 0.15f;

// Jump state
bool isJumping = false;
float jumpHeight = 2.0f;
float jumpVelocity = 5.0f;
float gravity = 9.8f;
float initialYPosition = 0.0f;

// Material properties
float materialShininess = 32.0f;

// Point Light variables
float ambientStrength = 0.05f;
float diffuseStrength = 0.8f;
float specularStrength = 1.0f;

// Directional light variables
glm::vec3 direction{ 0.3f, -0.7f, -0.4f };
float dirAmbient = 0.05f;
float dirDiffuse = 0.4f;
float dirSpecular = 0.5f;

// Flashlight toggle
bool useFlashlight = false;

// Timing
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// Model folder
std::vector<std::string> modelFolders;
static int selectedModelIdx = 0;

// Model container
std::vector<Model> allModels;

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
	glfwWindowHint(GLFW_SAMPLES, 4);

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

	glfwSetJoystickCallback([](int jid, int event) {
	if (jid == GLFW_JOYSTICK_1)
	{
		if (event == GLFW_CONNECTED)
		{
			std::cout << "Xbox controller connected" << std::endl;
			controllerConnected = true;
		}
		else if (event == GLFW_DISCONNECTED)
		{
			std::cout << "Xbox controller disconnected" << std::endl;
			controllerConnected = false;
		}
	}
	});

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

	// MSAA
	glEnable(GL_MULTISAMPLE);

	// Gamma Correction
	// glEnable(GL_FRAMEBUFFER_SRGB);

	bool wireframe = false;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	Shader blinnPhongShading("shaders/blinnPhong.vert", "shaders/blinnPhong.frag");
	Shader lightSource("shaders/lightSource.vert", "shaders/lightSource.frag");
	Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");

	Shader shadowMap("shaders/shadowMap.vert", "shaders/shadowMap.frag");	
	Shader debugDepthQuad("shaders/debugQuad.vert", "shaders/debugQuad.frag");

	float skyboxVertices[] = {
		// positions          
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

	uint32_t skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	std::vector<std::string> faces
	{
		"assets/textures/forest/posx.jpg",
		"assets/textures/forest/negx.jpg",
		"assets/textures/forest/posy.jpg",
		"assets/textures/forest/negy.jpg",
		"assets/textures/forest/posz.jpg",
		"assets/textures/forest/negz.jpg",
	};
	uint32_t cubemapTexture = loadCubemap(faces);

	// Configure depth map FBO
	const uint32_t SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
	uint32_t depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	// Create depth texture
	uint32_t depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	skyboxShader.use();
	glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);

	debugDepthQuad.use();
	glUniform1i(glGetUniformLocation(debugDepthQuad.ID, "depthMap"), 0);

	//stbi_set_flip_vertically_on_load(true);
	Model lightSourceSphere("assets/models/icoSphere/icoSphere.obj", false, "lightSource");

	std::vector<glm::vec3> pointLightPositions = {};
	const int MAX_POINT_LIGHTS = 10;

	// IMGUI Initialization
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	enum ShadingMode {BLINNPHONG};
	ShadingMode currentShadingMode = BLINNPHONG;

	bool drawModel = true;
	LoadModelFolders();

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

		processControllerInput();
		processInput(window);

		// Physics update
		if (isJumping)
		{
			// Apply velocity to position
			camera.Position.y += jumpVelocity * deltaTime;

			// Apply gravity to velocity
			jumpVelocity -= gravity * deltaTime;

			// Check if landed
			if (camera.Position.y <= initialYPosition)
			{
				camera.Position.y = initialYPosition;
				isJumping = false;
			}
		}

		glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 500.0f, 0.0f));
		ImGui::SetNextWindowSize(ImVec2(500.0f, io.DisplaySize.y));

		ImGuiWindowFlags window_flags = 
			ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | 
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | 
			ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("DockSpace Window", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		// Create a dock space inside this window
		ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

		ImGui::End();

		// 1.) Render depth of scene to texture (from light's perspective)
		glm::mat4 lightProjection;
		glm::mat4 lightView;
		glm::mat4 lightSpaceMatrix;
		glm::vec3 lightPos = -direction * 10.0f;
	
		float near_plane = 1.0f, far_plane = 50.0f;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		// Render scene from lights POV
		shadowMap.use();
		glUniformMatrix4fv(glGetUniformLocation(shadowMap.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		// render scene
		for (const Model& currModel : allModels)
		{
			if (currModel.visible) {
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, currModel.position);
				model = glm::rotate(model, glm::radians(currModel.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, glm::radians(currModel.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, glm::radians(currModel.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(currModel.scale));

				glUniformMatrix4fv(glGetUniformLocation(shadowMap.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
				currModel.Draw(shadowMap);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Reset viewport
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//2.) Render Scene as normal using the generated depth / shadow map
		Shader* activeShader;
		switch(currentShadingMode) 
		{
			case BLINNPHONG:
				activeShader = &blinnPhongShading;
				break;
			default:
				activeShader = &blinnPhongShading;
				break;
		}

		activeShader->use();
		glUniform3fv(glGetUniformLocation(activeShader->ID, "viewPos"), 1, glm::value_ptr(camera.Position));
		glUniformMatrix4fv(glGetUniformLocation(activeShader->ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

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
		glUniform1i(glGetUniformLocation(activeShader->ID, "NR_POINT_LIGHTS"), pointLightPositions.size());
		for (uint32_t i = 0; i < pointLightPositions.size(); ++i)
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

		// Spot light
		glUniform1i(glGetUniformLocation(activeShader->ID, "enableSpotLight"), useFlashlight ? 1 : 0);
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

		// View / Projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(activeShader->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(activeShader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glm::vec3 defaultColor{ 0.8f,0.8f,0.8f };
		glUniform3fv(glGetUniformLocation(activeShader->ID, "defaultColor"), 1, glm::value_ptr(defaultColor));

		// Use higher texture unit to not overlap Diffuse and Specular
		glUniform1i(glGetUniformLocation(activeShader->ID, "shadowMap"), 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		// Render all models
		for (const Model& currModel : allModels)
		{
			if (currModel.visible) {
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, currModel.position);
				model = glm::rotate(model, glm::radians(currModel.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, glm::radians(currModel.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, glm::radians(currModel.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(currModel.scale));

				glUniformMatrix4fv(glGetUniformLocation(activeShader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
				currModel.Draw(*activeShader);
			}
		}

		lightSource.use();
		glUniformMatrix4fv(glGetUniformLocation(lightSource.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(lightSource.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));

		for (uint32_t i = 0; i < pointLightPositions.size(); i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f));
			glUniformMatrix4fv(glGetUniformLocation(lightSource.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));

			lightSourceSphere.Draw(lightSource);
		}

		// Draw skybox last in the scene
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // Remove translation from the view matrix
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // Reset depth function

		// Render depth map to quad for visual debugging
		// debugDepthQuad.use();
		// glUniform1f(glGetUniformLocation(debugDepthQuad.ID, "near_plane"), near_plane);
		// glUniform1f(glGetUniformLocation(debugDepthQuad.ID, "far_plane"), far_plane);
		// glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, depthMap);
		// renderQuad();

		ImGui::Begin("OGLRenderer Interface");
		ImGui::Text("Scene Construction");
		ImGui::Separator();

		if (ImGui::BeginCombo("Select Model", modelFolders[selectedModelIdx].c_str())) {
			// Iterate through modelFolders and display each item
			for (int i = 0; i < modelFolders.size(); ++i) {
				bool isSelected = (selectedModelIdx == i);
				if (ImGui::Selectable(modelFolders[i].c_str(), isSelected)) {
					selectedModelIdx = i;  // Update the selection
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Add Model")) {
			std::string selectedFolder = modelFolders[selectedModelIdx];
			std::string modelPath = "assets/models/" + selectedFolder + "/" + selectedFolder + ".obj";
			allModels.emplace_back(Model(modelPath, false, selectedFolder));
		}

		ImGui::Separator();
		ImGui::Text("Modify Model Properties");

		if (ImGui::CollapsingHeader("Models"))
		{
			for (int i = 0; i < allModels.size(); ++i)
			{
				Model& model = allModels[i];

				ImGui::PushID(i);
				if (ImGui::TreeNode(model.name.c_str()))
				{
					ImGui::Checkbox("Visible", &model.visible);
					ImGui::SliderFloat("Scale", &model.scale, 0.01f, 2.0f);
					ImGui::DragFloat3("Model Position", glm::value_ptr(model.position), 0.1f);
					ImGui::SliderFloat("Model Rotation X", &model.rotation.x, 0.0f, 360.0f);
					ImGui::SliderFloat("Model Rotation Y", &model.rotation.y, 0.0f, 360.0f);
					ImGui::SliderFloat("Model Rotation Z", &model.rotation.z, 0.0f, 360.0f);

					// Remove Button for each model
					if (ImGui::Button("Remove Model")) {
						// Find the position of the first non-digit character from the end of the model name
						size_t lastDigitPos = model.name.find_last_not_of("0123456789");

						// Extract the base name up to that position (if there are any digits at the end)
						std::string baseName = model.name.substr(0, lastDigitPos + 1); // lastDigitPos + 1 to include the part before digits

						auto it = Model::modelNameCount.find(baseName);
						if (it != Model::modelNameCount.end()) {
							it->second--;  // Decrease the count
							if (it->second == 0) 
							{
								Model::modelNameCount.erase(it);  // Optionally, remove the entry if the count reaches 0
							}
						}

						// Use the full model name to find and remove the model
						allModels.erase(std::remove_if(allModels.begin(), allModels.end(),
							[&model](const Model& m) {
								return m.name == model.name; // Match by full name
							}), allModels.end());
					}

					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}

		ImGui::Separator();
		ImGui::Text("Specular Exponent");
		ImGui::SliderFloat("Shininess", &materialShininess, 1.0f, 256.0f);

		ImGui::Separator();
		ImGui::Text("Point Light Properties");
		ImGui::SliderFloat("Ambient Strength", &ambientStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("Diffuse Strength", &diffuseStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("Specular Strength", &specularStrength, 0.0f, 1.0f);

		ImGui::Separator();
		ImGui::Text("Active Point Lights: %zu/%d", pointLightPositions.size(), MAX_POINT_LIGHTS);

		if (ImGui::Button("Add Light") && pointLightPositions.size() < MAX_POINT_LIGHTS)
		{
			// Add a new light at a default position near the camera
			glm::vec3 newLightPos{ 0.0f, 0.0f, 0.0f };
			pointLightPositions.push_back(newLightPos);
		}

		ImGui::SameLine();
		if (ImGui::Button("Remove Light"))
		{
			pointLightPositions.pop_back();
		}

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

		ImGui::Separator();
		ImGui::Text("Controller Status");
		ImGui::Text("Controller %s", controllerConnected ? "Connected" : "Disconnected");
		if (controllerConnected)
		{
			ImGui::Text("Left Stick: X=%.2f, Y=%.2f", leftStickX, leftStickY);
			ImGui::Text("Right Stick: X=%.2f, Y=%.2f", rightStickX, rightStickY);
			ImGui::Text("Triggers: L=%.2f, R=%.2f", leftTrigger, rightTrigger);
			ImGui::Text("Buttons: A=%d, B=%d, X=%d, Y=%d", aButtonPressed, bButtonPressed, xButtonPressed, yButtonPressed);
		}

		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

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

	// Process controller inputs
	if (controllerConnected)
	{
		// Left stick for movement (up/down/left/right)
		if (leftStickY < -CONTROLLER_DEADZONE)
			camera.ProcessKeyboard(FORWARD, deltaTime * std::abs(leftStickY));
		if (leftStickY > CONTROLLER_DEADZONE)
			camera.ProcessKeyboard(BACKWARD, deltaTime * leftStickY);
		if (leftStickX < -CONTROLLER_DEADZONE)
			camera.ProcessKeyboard(LEFT, deltaTime * std::abs(leftStickX));
		if (leftStickX > CONTROLLER_DEADZONE)
			camera.ProcessKeyboard(RIGHT, deltaTime * leftStickX);
	}

	if (std::abs(rightStickX) > CONTROLLER_DEADZONE || std::abs(rightStickY) > CONTROLLER_DEADZONE)
	{
		float sensitivity = 2.0f; // adjust to control rotation speed
		camera.ProcessMouseMovement(rightStickX * sensitivity, -rightStickY * sensitivity);
	}

	// Bumpers for up/down movement or zoom
	if (leftBumper)
	{
		camera.Position.y -= cameraSpeed;
	}
	if (rightBumper)
	{
		camera.Position.y += cameraSpeed;
	}

	// use dpad for UI nav or other controls
	static bool yButtonPrevState = false;
	if (yButtonPressed && !yButtonPrevState)
	{
		useFlashlight = !useFlashlight;
	}
	yButtonPrevState = yButtonPressed;

	static bool aButtonPrevState = false;
	if (aButtonPressed && !aButtonPrevState && !isJumping)
	{
		isJumping = true;
		jumpVelocity = 5.0f;
	}
	aButtonPrevState = aButtonPressed;
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

void processControllerInput()
{
	controllerConnected = glfwJoystickPresent(GLFW_JOYSTICK_1);

	if (controllerConnected)
	{
		int axisCount;
		const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axisCount);

		int buttonCount;
		const uint8_t* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);

		if (axisCount >= 6 && buttonCount >= 14)
		{
			// Xbox 360 controller layout in GLFW
			// Get analog stick values and apply deadzone
			leftStickX = std::abs(axes[0]) > CONTROLLER_DEADZONE ? axes[0] : 0.0f;
			leftStickY = std::abs(axes[1]) > CONTROLLER_DEADZONE ? axes[1] : 0.0f;
			rightStickX = std::abs(axes[2]) > CONTROLLER_DEADZONE ? axes[2] : 0.0f;
			rightStickY = std::abs(axes[3]) > CONTROLLER_DEADZONE ? axes[3] : 0.0f;

			// Triggers (range from -1 to 1, convert to 0 or 1)
			leftTrigger = (axes[4] + 1.0f) * 0.5f;
			rightTrigger = (axes[5] + 1.0f) * 0.5f;

			// Button states
			aButtonPressed = buttons[0] == GLFW_PRESS;
			bButtonPressed = buttons[1] == GLFW_PRESS;
			xButtonPressed = buttons[2] == GLFW_PRESS;
			yButtonPressed = buttons[3] == GLFW_PRESS;
			leftBumper = buttons[4] == GLFW_PRESS;
			rightBumper = buttons[5] == GLFW_PRESS;

			// D-Pad states
			dpadUp = buttons[10] == GLFW_PRESS;
			dpadRight = buttons[11] == GLFW_PRESS;
			dpadDown = buttons[12] == GLFW_PRESS;
			dpadLeft = buttons[13] == GLFW_PRESS;
		}
	}
}

uint32_t loadCubemap(std::vector<std::string> faces)
{
	uint32_t textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (uint32_t i = 0; i < faces.size(); ++i)
	{
		uint8_t* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void LoadModelFolders()
{
	modelFolders.clear();
	for (const auto& entry : std::filesystem::directory_iterator("assets/models"))
	{
		if (entry.is_directory())
		{
			for (const auto& file : std::filesystem::directory_iterator(entry.path()))
			{
				if (file.path().extension() == ".obj") 
				{
					modelFolders.push_back(entry.path().filename().string());
					break;
				}
			}
		}
	}
}