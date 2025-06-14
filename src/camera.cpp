#include "camera.hpp"
#include "physics.hpp"
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>

Camera::Camera(glm::vec3 position, float yaw, float pitch)
    : Position(position), Yaw(yaw), Pitch(pitch)
{
    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    glm::vec3 newPosition = Position;
    
    switch (direction)
    {
        case FORWARD: newPosition += Front * velocity; break;
        case BACKWARD: newPosition -= Front * velocity; break;
        case LEFT: newPosition -= Right * velocity; break;
        case RIGHT: newPosition += Right * velocity; break;
    }
    
    // Only move if there's no collision
    if (!CheckCollision(newPosition))
    {
        Position = newPosition;
    }
}

bool Camera::CheckCollision(const glm::vec3& newPosition) const
{
    if (!physicsSystem) return false;
    
    // Use a capsule shape for the camera collision
    JPH::RefConst<JPH::Shape> capsuleShape = new JPH::CapsuleShape(collisionHeight * 0.5f, collisionRadius);
    
    // Convert new position to Jolt types
    JPH::RVec3 testPos(newPosition.x, newPosition.y, newPosition.z);
    
    // Create transformation matrix for the test position
    JPH::RMat44 transform = JPH::RMat44::sTranslation(testPos);
    
    // Create a simple collision collector that stops at first hit
    class FirstHitCollector : public JPH::CollideShapeCollector
    {
    public:
        bool mHadHit = false;
        
        virtual void AddHit(const JPH::CollideShapeResult &inResult) override
        {
            mHadHit = true;
            ForceEarlyOut(); // Stop after first hit
        }
        
        bool HadHit() const { return mHadHit; }
    };
    
    FirstHitCollector collector;
    
    // Perform collision test at the new position
    physicsSystem->GetNarrowPhaseQuery().CollideShape(
        capsuleShape,           // Shape to test
        JPH::Vec3::sReplicate(1.0f), // Scale
        transform,              // Transform
        JPH::CollideShapeSettings(), // Settings
        JPH::RVec3::sZero(),    // Base offset
        collector               // Collector
    );
    
    return collector.HadHit();
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset)
{
    xOffset *= MouseSensitivity;
    yOffset *= MouseSensitivity;
    
    Yaw += xOffset;
    Pitch += yOffset;
    
    if (Pitch > 89.0f) Pitch = 89.0f;
    else if (Pitch < -89.0f) Pitch = -89.0f;
    
    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yOffset)
{
    Zoom -= yOffset;
    if (Zoom < 1.0) Zoom = 1.0f;
    else if (Zoom > 90.0f) Zoom = 90.0f;
}

void Camera::UpdateCameraVectors()
{
    float yawRad = glm::radians(Yaw);
    float pitchRad = glm::radians(Pitch);
    Front = glm::normalize(glm::vec3(
        cos(yawRad) * cos(pitchRad),
        sin(pitchRad),
        sin(yawRad) * cos(pitchRad)
    ));
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
