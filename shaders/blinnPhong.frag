#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float shininess;
}; 

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

#define MAX_POINT_LIGHTS 10

in vec2 TexCoords;
in vec3 TangentViewPos;
in vec3 TangentFragPos;
in vec3 WorldNormal;
in vec4 FragPosLightSpace;
in mat3 TBN_inverse;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int NR_POINT_LIGHTS;
uniform SpotLight spotLight;
uniform Material material;
uniform sampler2D shadowMap;

uniform bool hasDiffuse;
uniform bool hasSpecular;
uniform bool hasNormal;
uniform bool useNormalMaps;

uniform bool enableSpotLight;
uniform vec3 defaultColor;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 getDiffuseColor();
vec3 getSpecularColor();

float shadowCalculation(vec4 FragPosLightSpace);

void main()
{    
    // properties
    vec3 norm;
    if (hasNormal && useNormalMaps)
    {
        // Sample normal map and transform to [-1, 1] range
        vec3 normalMap = texture(material.texture_normal1, TexCoords).rgb;
        normalMap = normalMap * 2.0 - 1.0;

        // Invert Y component to convert from DirectX to OpenGL format
        normalMap.y = -normalMap.y;

        norm = normalize(normalMap);
    } else {
        // Transform World normal to Tangent space if there is no normal map
        norm = normalize(TBN_inverse * WorldNormal);
    }

    // Calculate the view direction in Tangent space
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    
    // == =====================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == =====================================================
    // phase 1: directional lighting
    // The dirLight.direction needs to be transformed to tangent space
    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    // phase 2: point lights
    for (int i = 0; i < NR_POINT_LIGHTS; i++) 
    {
        // Trasform point light position to tangent space in the function
        result += CalcPointLight(pointLights[i], norm, TangentFragPos, viewDir);    
    }

    // phase 3: spot light
    if (enableSpotLight) {
        // Transform spot light position to tangent space in the function
        result += CalcSpotLight(spotLight, norm, TangentFragPos, viewDir);    
    }

    FragColor = vec4(result, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 TangentLightDir = normalize(TBN_inverse * (-light.direction));
    // diffuse shading
    float diff = max(dot(normal, TangentLightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(TangentLightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess * 4);
    // combine results
    vec3 ambient = light.ambient * getDiffuseColor();
    vec3 diffuse = light.diffuse * diff * getDiffuseColor();
    vec3 specular = light.specular * spec * getSpecularColor();

    // calculate shadows
    float shadow = shadowCalculation(FragPosLightSpace);
    
    // apply shadow to diffuse and specular
    diffuse *= (1.0 - shadow);
    specular *= (1.0 - shadow);
    
    return (ambient + diffuse + specular);  
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 tangentFragPos, vec3 viewDir)
{
    // Transform light position to tangent space
    vec3 tangentLightPos = TBN_inverse * light.position;
    vec3 lightDir = normalize(tangentLightPos - tangentFragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess * 4);
    // attenuation
    float distance = length(tangentLightPos - tangentFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * getDiffuseColor();
    vec3 diffuse = light.diffuse * diff * getDiffuseColor();
    vec3 specular = light.specular * spec * getSpecularColor();

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 tangentFragPos, vec3 viewDir)
{
    // Transform spot light position and direction to tangent space
    vec3 tangentLightPos = TBN_inverse * light.position;
    vec3 tangentLightDir = normalize(TBN_inverse * (-light.direction));

    vec3 lightDir = normalize(tangentLightPos - tangentFragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(tangentLightPos - tangentFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-tangentLightDir)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * getDiffuseColor();
    vec3 diffuse = light.diffuse * diff * getDiffuseColor();
    vec3 specular = light.specular * spec * getSpecularColor();

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}

vec3 getDiffuseColor() {
    return hasDiffuse ? texture(material.texture_diffuse1, TexCoords).rgb : defaultColor;
}

vec3 getSpecularColor() {
    return hasSpecular ? texture(material.texture_specular1, TexCoords).rgb : vec3(0.2);
}

float shadowCalculation(vec4 FragPosLightSpace)
{
    // Perform perspective divide
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // calculate bias (based on depth map resolution and slope) on world space normals
    float bias = max(0.05 * (1.0 - dot(normalize(WorldNormal), normalize(-dirLight.direction))), 0.005);

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}