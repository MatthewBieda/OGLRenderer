#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in mat4 aInstanceMatrix;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out vec4 FragPosLightSpace;
out mat3 TBN;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 lightSpaceMatrix;

void main()
{
    // Calculate world position
    vec4 worldPos = aInstanceMatrix * vec4(aPos, 1.0);
    WorldPos = worldPos.xyz;

    // Calculate normal in world space (support non-uniform scale)
    mat3 normalMatrix = mat3(transpose(inverse(aInstanceMatrix))); 
    Normal = normalize(normalMatrix * aNormal);

    // Calculate TBN matrix for normal mapping
    vec3 T = normalize(normalMatrix * aTangent);

    // Re-orthogalize T with respect to N (Gram-Schmidt process)
    T = normalize(T - dot(T, Normal) * Normal);
    // Calculate bitangent
    vec3 B = cross(Normal, T);

    // TBN matrix for transforming from tangent to world space
    TBN = mat3(T, B, Normal);

    // Transform position to light space for shadow mapping
    FragPosLightSpace = lightSpaceMatrix * worldPos;

    // Pass texture co-ordinates
    TexCoords = aTexCoords;

    // Output clip space position
    gl_Position = projection * view * worldPos;
}

// Optimise for uniform scaling later
// mat3 normalMatrix = mat3(aInstanceMatrix); // Extract upper 3x3
// Normal = normalMatrix * aNormal;