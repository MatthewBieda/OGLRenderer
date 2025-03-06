#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // Transform vertex position to view space
    vec3 FragPos = vec3(view * model * vec4(aPos, 1.0));
    
    // Transform normal to view space (ensure correct transformation matrix)
    vec3 Normal = mat3(transpose(inverse(view * model))) * aNormal;
    Normal = normalize(Normal);
    
    // Transform light position to view space
    vec3 LightPos = vec3(view * vec4(lightPos, 1.0));
    
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    // diffuse
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(-FragPos); 
    vec3 reflectDir = reflect(-lightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    // Calculate final color at the vertex
    FragColor = (ambient + diffuse + specular) * objectColor;
}