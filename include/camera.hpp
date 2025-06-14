#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Jolt/Jolt.h>

// Camera movement enums
enum Camera_Movement 
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
constexpr float YAW = -90.0f;
constexpr float PITCH = 0.0f;
constexpr float SPEED = 2.5f;
constexpr float SENSITIVITY = 0.2f;
constexpr float ZOOM = 60.0f;
static constexpr glm::vec3 WorldUp{ 0.0f, 1.0f, 0.0f };

// Camera class that processes input and calculates Euler Angles, Vectors, and Matrices
struct Camera
{
    // Camera Attributes
    glm::vec3 Position{ 0.0f, 2.0f, 8.0f };
    glm::vec3 Front{ 0.0f, 0.0f, -1.0f };
    glm::vec3 Up{ 0.0f, 1.0f, 0.0f };
    glm::vec3 Right{ 1.0f, 0.0f, 0.0f };
    
    // Euler Angles
    float Yaw{ YAW };
    float Pitch{ PITCH };
    
    // Camera options
    float MovementSpeed{ SPEED };
    float MouseSensitivity{ SENSITIVITY };
    float Zoom{ ZOOM };
    
    // Simple collision properties
    float collisionRadius{ 0.3f };
    float collisionHeight{ 1.8f };
    
    // Default Constructor
    Camera(glm::vec3 position = {0.0f, 2.0f, 8.0f}, float yaw = YAW, float pitch = PITCH);
    
    // Existing methods
    glm::mat4 GetViewMatrix() const;
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(float xOffset, float yOffset);
    void ProcessMouseScroll(float yOffset);
    void UpdateCameraVectors();
    
    // Simple collision method
    bool CheckCollision(const glm::vec3& newPosition) const;
};
