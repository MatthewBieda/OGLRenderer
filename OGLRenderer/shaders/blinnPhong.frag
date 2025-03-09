#version 330 core
out vec4 FragColor;

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

// Take in all info for lighting calculations in view space!
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 LightPos; 

uniform Material material;
uniform Light light;

void main()
{
    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
    
     // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;
    
    // specular - Blinn-Phong
    vec3 viewDir = normalize(-FragPos); // the viewer is always at (0,0,0) in view-space
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess * 4);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}