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

class GameObject {
public:
	std::shared_ptr<Model> model;  // Using shared_ptr for better memory management
	glm::vec3 position = { 0, 0, 0 };
	glm::vec3 rotation = { 0, 0, 0 };
	float scale = 1.0f;
	bool visible = true;
	std::string name;

	GameObject(std::shared_ptr<Model> model, const std::string& name) : model(model), name(name) {}

	glm::mat4 getTransformMatrix() const {
		glm::mat4 transform(1.0f);
		transform = glm::translate(transform, position);
		transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1, 0, 0));
		transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0, 1, 0));
		transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0, 0, 1));
		transform = glm::scale(transform, glm::vec3(scale));
		return transform;
	}
};

unsigned int loadCubemap(std::vector<std::string> faces);

// Screen 
int SCR_WIDTH = 2560;
int SCR_HEIGHT = 1440;

static const std::array<std::pair<int, int>, 4> resolutions
{
	{
		{1280, 720},
		{1920, 1080},
		{2560, 1440},
		{3840, 2160}
	}
};

enum class DisplayMode
{
	Windowed,
	Fullscreen,
	Borderless
};

DisplayMode currentDisplayMode = DisplayMode::Windowed;

void SetDisplayMode(GLFWwindow* window, DisplayMode mode);

int windowedX = 100, windowedY = 100;
int windowedWidth = SCR_WIDTH, windowedHeight = SCR_HEIGHT;

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

// Light Properties 
glm::vec3 direction{ 0.3f, -0.7f, -0.4f };
glm::vec3 sunLightColor = glm::vec3(5.0f, 4.9f, 4.75f); // Half intensity but same color temperature
//glm::vec3 sunLightColor = glm::vec3(10.0f, 9.8f, 9.5f); // Slightly warm sunlight
// or higher for bright daylight: glm::vec3(20.0f, 19.5f, 19.0f);

glm::vec3 pointLightColor = glm::vec3(5.0f, 4.8f, 4.5f); // Slightly warm indoor light
// For a warm/yellowish bulb: glm::vec3(5.0f, 4.0f, 2.5f);
// For a cool/bluish light: glm::vec3(3.0f, 3.5f, 5.0f);

// For flashlight/spotlight
glm::vec3 spotlightColor = glm::vec3(4.0f); // White light

// Flashlight toggle
bool useFlashlight = false;

// Timing
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// Model folder
std::vector<std::string> modelFolders;
static int selectedModelIdx = 0;

// Game object container
std::vector<GameObject> gameObjects;
std::unordered_map<std::string, std::shared_ptr<Model>> modelCache;

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
	bool useNormalMaps = true;
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

		// Group transforms by model for shadow pass
		std::unordered_map<std::shared_ptr<Model>, std::vector<glm::mat4>> batchedInstanceData;

		// Group transforms by model
		for (const auto& obj : gameObjects) 
		{
			if (obj.visible) {
				batchedInstanceData[obj.model].push_back(obj.getTransformMatrix());
			}
		}

		// Render each model with all its instances for shadow mapping
		for (auto& [modelPtr, transforms] : batchedInstanceData) 
		{
			// Skip if no visible instances
			if (transforms.empty()) continue;

			// Update model's instance buffer
			glBindBuffer(GL_ARRAY_BUFFER, modelPtr->instanceVBO);
			glBufferData(GL_ARRAY_BUFFER,
				transforms.size() * sizeof(glm::mat4),
				transforms.data(),
				GL_DYNAMIC_DRAW);

			// Draw the model with instancing for shadows
			modelPtr->Draw(shadowMap, transforms.size());
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
		glUniform3fv(glGetUniformLocation(activeShader->ID, "camPos"), 1, glm::value_ptr(camera.Position));
		glUniformMatrix4fv(glGetUniformLocation(activeShader->ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

		// Set Material Properties
		activeShader->setBool("useNormalMaps", useNormalMaps);

		// Directional Light
		glUniform3fv(glGetUniformLocation(activeShader->ID, "dirLight.direction"), 1, glm::value_ptr(direction));
		glUniform3fv(glGetUniformLocation(activeShader->ID, "dirLight.color"), 1, glm::value_ptr(sunLightColor));

		// Point Lights
		glUniform1i(glGetUniformLocation(activeShader->ID, "NR_POINT_LIGHTS"), pointLightPositions.size());
		for (uint32_t i = 0; i < pointLightPositions.size(); ++i)
		{
			std::string number = std::to_string(i);

			glUniform3fv(glGetUniformLocation(activeShader->ID, ("pointLights[" + number + "].position").c_str()), 1, glm::value_ptr(pointLightPositions[i]));
			glUniform3fv(glGetUniformLocation(activeShader->ID, ("pointLights[" + number + "].color").c_str()), 1, glm::value_ptr(pointLightColor));
		}

		// Spot light
		glUniform1i(glGetUniformLocation(activeShader->ID, "enableSpotLight"), useFlashlight ? 1 : 0);
		glUniform3fv(glGetUniformLocation(activeShader->ID, "spotLight.position"), 1, glm::value_ptr(camera.Position));
		glUniform3fv(glGetUniformLocation(activeShader->ID, "spotLight.direction"), 1, glm::value_ptr(camera.Front));
		glUniform3fv(glGetUniformLocation(activeShader->ID, "spotLight.color"), 1, glm::value_ptr(spotlightColor));
		glUniform1f(glGetUniformLocation(activeShader->ID, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
		glUniform1f(glGetUniformLocation(activeShader->ID, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));

		// View / Projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(activeShader->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(activeShader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// Default PBR values
		glm::vec3 defaultAlbedo = glm::vec3(0.8f);
		float defaultMetallic = 0.0f;
		float defaultRoughness = 0.5;
		float defaultAO = 1.0f;

		glUniform3fv(glGetUniformLocation(activeShader->ID, "defaultAlbedo"), 1, glm::value_ptr(defaultAlbedo));
		activeShader->setFloat("defaultMetallic", defaultMetallic);
		activeShader->setFloat("defaultRoughness", defaultRoughness);
		activeShader->setFloat("defaultAO", defaultAO);

		// Use texture unit 5 for shadow map to allow room for albedo/normals/metallic/roughness/ao
		glUniform1i(glGetUniformLocation(activeShader->ID, "shadowMap"), 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		// Render each model with all its instances
		for (auto& [modelPtr, transforms] : batchedInstanceData) { 
			// Skip if no visible instances or null model
			if (!modelPtr || transforms.empty()) {
				std::cout << "Skipping model - null or no transforms" << std::endl;
				continue;
			}

			// No need to set model matrix uniform when instancing
			modelPtr->Draw(*activeShader, transforms.size());
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

			lightSourceSphere.Draw(lightSource, 1);
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

		static int instanceCount = 1;

		ImGui::InputInt("Instances to Add", &instanceCount);
		if (instanceCount < 1) 
		{
			instanceCount = 1;
		}

		// Update your "Add Model" button handler
		if (ImGui::Button("Add Model")) {
			std::string selectedFolder = modelFolders[selectedModelIdx];
			std::string modelPath = "assets/models/" + selectedFolder + "/" + selectedFolder + ".obj";

			// Check if we already have this model in cache
			std::shared_ptr<Model> modelPtr;

			if (modelCache.find(selectedFolder) == modelCache.end()) {
				// Create new model and add to cache
				modelPtr = std::make_shared<Model>(modelPath, false, selectedFolder);
				modelCache[selectedFolder] = modelPtr;
				std::cout << "Created new model: " << selectedFolder << std::endl;
			}
			else {
				// Use existing model from cache
				modelPtr = modelCache[selectedFolder];
				std::cout << "Using cached model: " << selectedFolder << std::endl;
			}

			// Add multiple GameObjects
			int gridSize = static_cast<int>(std::sqrt(instanceCount)); // e.g. 10 for 100 instances
			float spacing = 2.5f; 

			for (int i = 0; i < instanceCount; ++i) {
				std::string objName = selectedFolder + "_" + std::to_string(gameObjects.size());

				GameObject obj(modelPtr, objName);

				int row = i / gridSize;
				int col = i % gridSize;

				obj.position = glm::vec3(col * spacing, 0.0f, row * spacing);

				gameObjects.push_back(std::move(obj));
			}
			std::cout << "Added " << instanceCount << " instances of " << selectedFolder << std::endl;

		}

		ImGui::Separator();
		ImGui::Text("Modify Model Properties");

		if (ImGui::CollapsingHeader("GameObjects"))
		{
			for (int i = 0; i < gameObjects.size(); ++i)
			{
				GameObject& obj = gameObjects[i];
				ImGui::PushID(i);
				if (ImGui::TreeNode(obj.name.c_str()))
				{
					ImGui::Checkbox("Visible", &obj.visible);
					ImGui::SliderFloat("Scale", &obj.scale, 0.01f, 2.0f);
					ImGui::DragFloat3("Position", glm::value_ptr(obj.position), 0.1f);
					ImGui::SliderFloat("Rotation X", &obj.rotation.x, 0.0f, 360.0f);
					ImGui::SliderFloat("Rotation Y", &obj.rotation.y, 0.0f, 360.0f);
					ImGui::SliderFloat("Rotation Z", &obj.rotation.z, 0.0f, 360.0f);

					// Remove Button for each GameObject
					if (ImGui::Button("Remove GameObject")) {
						gameObjects.erase(gameObjects.begin() + i);
						i--; // Adjust index since we removed an element
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}

		ImGui::Separator();
		ImGui::Checkbox("Enable Normals Maps", &useNormalMaps);

		ImGui::Separator();
		ImGui::Text("Active Point Lights: %zu/%d", pointLightPositions.size(), MAX_POINT_LIGHTS);

		if (ImGui::Button("Add Light") && pointLightPositions.size() < MAX_POINT_LIGHTS)
		{
			// Add a new light at a default position near the camera
			glm::vec3 newLightPos{ 0.0f, 0.0f, 0.0f };
			pointLightPositions.push_back(newLightPos);
		}

		ImGui::SameLine();
		if (ImGui::Button("Remove Light") && !pointLightPositions.empty())
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

		if (ImGui::Begin("Display Settings"))
		{
			static int selectedRes = 1;
			static int selectedDisplayMode = 0;

			const char* resLabels[] = { "1280x720", "1920x1080", "2560x1440", "3840x2160" };
			const char* displayLabels[] = { "Windowed", "Exclusive Fullscreen", "Borderless Fullscreen" };

			// Sync the selected resolution to current window size
			for (int i = 0; i < resolutions.size(); ++i)
			{
				if (SCR_WIDTH == resolutions[i].first && SCR_HEIGHT == resolutions[i].second)
				{
					selectedRes = i;
					break;
				}
			}

			bool isWindowed = currentDisplayMode == DisplayMode::Windowed;
			ImGui::BeginDisabled(!isWindowed);
			if (ImGui::Combo("Resolution", &selectedRes, resLabels, IM_ARRAYSIZE(resLabels)))
			{
				int newWidth = resolutions[selectedRes].first;
				int newHeight = resolutions[selectedRes].second;

				glfwSetWindowSize(window, newWidth, newHeight);
				SCR_WIDTH = newWidth;
				SCR_HEIGHT = newHeight;
			}
			ImGui::EndDisabled();

			if (ImGui::Combo("Display Mode", &selectedDisplayMode, displayLabels, IM_ARRAYSIZE(displayLabels)))
			{
				SetDisplayMode(window, static_cast<DisplayMode>(selectedDisplayMode));
			}
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
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	glViewport(0, 0, width, height);
}

void SetDisplayMode(GLFWwindow* window, DisplayMode mode)
{
	if (mode == currentDisplayMode)
	{
		return;
	}

	if (currentDisplayMode == DisplayMode::Windowed)
	{
		glfwGetWindowPos(window, &windowedX, &windowedY);
		glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* modeInfo = glfwGetVideoMode(monitor);

	switch (mode)
	{
	case DisplayMode::Windowed:
		glfwSetWindowMonitor(window, nullptr, windowedX, windowedY, windowedWidth, windowedHeight, 0);
		glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
		break;

	case DisplayMode::Fullscreen:
		glfwSetWindowMonitor(window, monitor, 0, 0, modeInfo->width, modeInfo->height, modeInfo->refreshRate);
		break;
	
	case DisplayMode::Borderless:
		glfwSetWindowMonitor(window, nullptr, 0, 0, modeInfo->width, modeInfo->height, 0);
		glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
		framebuffer_size_callback(window, modeInfo->width, modeInfo->height);

		break;
	}


	currentDisplayMode = mode;
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

//TODO
//Move constant light direction transformations to the vertex shader:

//Transform directional light direction once
//Transform spotlight direction once
//Pass as varyings to the fragment shader


//Consider transforming point light positions in the vertex shader too (if you have only a few points lights)