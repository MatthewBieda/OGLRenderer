#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in mat4 aInstanceMatrix; // Instance transform matrix

out vec2 TexCoords;
out vec3 TangentViewPos;
out vec3 TangentFragPos;
out vec3 WorldNormal;
out vec4 FragPosLightSpace;
out mat3 TBN_inverse; // TBN inverse for transforming to tangent space

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;
uniform vec3 viewPos;
uniform vec3 dirLightDirection;

// For point lights
#define MAX_POINT_LIGHTS 10
uniform vec3 pointLightPositions[MAX_POINT_LIGHTS];
uniform int numPointLights;

// For spotlight
uniform vec3 spotLightPosition;
uniform vec3 spotLightDirection;
uniform bool enableSpotLight;

void main()
{
    // Calculate world position
    vec4 worldPos = aInstanceMatrix * vec4(aPos, 1.0);
    vec3 worldPosition = worldPos.xyz;

    // Calculate normal, tangent, and bitangent in World space
    mat3 normalMatrix = mat3(transpose(inverse(aInstanceMatrix))); // Support non-uniform scaling
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent);

    // Re-orthogalize T with respect to N (Gram-Schmidt process)
    T = normalize(T - dot(T, N) * N);
    // Then reconstruct B to ensure orthoganal basis
    vec3 B = cross(N, T);

    // Create new TBN matrix (world space to tangent space)
    mat3 TBN = mat3(T, B, N);
    TBN_inverse = transpose(TBN); // Inverse for transforming World -> Tangent

    // Transform view position to tangent space
    TangentViewPos = TBN_inverse * viewPos;

    // Transoform fragment position to tangent space
    TangentFragPos = TBN_inverse * worldPosition;

    // Save world normal for shadow calculations
    WorldNormal = N;

    // Transform position to light space for shadow mapping
    FragPosLightSpace = lightSpaceMatrix * worldPos;

    TexCoords = aTexCoords;
    gl_Position = projection * view * worldPos;
}

// Optimise for uniform scaling later
// mat3 normalMatrix = mat3(aInstanceMatrix); // Extract upper 3x3
// Normal = normalMatrix * aNormal;