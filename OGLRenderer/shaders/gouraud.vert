#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragColor;

struct Material 
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light 
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

uniform vec3 lightPos;

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
    vec3 ambient = light.ambient * texture(material.diffuse, aTexCoords).rgb;
    
     // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, aTexCoords).rgb;
    
    // specular - Phong
    vec3 viewDir = normalize(-FragPos); // the viewer is always at (0,0,0) in view-space, 
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, aTexCoords).rgb;
    
    // Calculate final color at the vertex
    FragColor = ambient + diffuse + specular;
}