#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in vec4 FragPosLightSpace;
in mat3 TBN;

struct PBRMaterial {
    sampler2D albedoMap;
    sampler2D normalMap;
    sampler2D metallicRoughnessMap;
    sampler2D aoMap;
    sampler2D emissiveMap;
};

// Material properties
uniform bool hasAlbedo;
uniform bool hasNormal;
uniform bool hasMetallicRoughness;
uniform bool hasAO;
uniform bool hasEmissive;
uniform bool useNormalMaps;

uniform vec3 defaultAlbedo;
uniform float defaultMetallic;
uniform float defaultRoughness;
uniform float defaultAO;

// Light structures
struct DirLight {
    vec3 direction;
    vec3 color;
};

struct PointLight {
    vec3 position;
    vec3 color;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;

    float cutOff;
    float outerCutOff;
};

// Light Uniforms
#define MAX_POINT_LIGHTS 10
uniform DirLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int NR_POINT_LIGHTS;
uniform SpotLight spotLight;
uniform bool enableSpotLight;
uniform bool enableDirLight;
uniform vec3 camPos;
uniform PBRMaterial pbrMaterial;
uniform sampler2D shadowMap;

// IBL Uniforms
uniform samplerCube irradianceMap; // Diffuse environment lighting
uniform samplerCube prefilterMap; // Prefiltered environment map for specular
uniform sampler2D brdfLUT; // BRDF lookup texture
uniform bool useIBL;

// Constants
const float PI = 3.14159265359;
const float MAX_REFLECTION_LOD = 0.0; // Depending on mip levels of prefilterMap

// function prototypes
vec3 getNormalFromMap();
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRougness(float cosTheta, vec3 F0, float roughness);
float shadowCalculation(vec4 FragPosLightSpace);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0);
vec3 getDiffuseColor();
vec3 getSpecularColor();

void main()
{    
    // Simple material properties
    vec3 albedo = hasAlbedo ? texture(pbrMaterial.albedoMap, TexCoords).rgb : defaultAlbedo;
    vec4 metallicRoughness = hasMetallicRoughness ? texture(pbrMaterial.metallicRoughnessMap, TexCoords) : vec4(0.0, defaultRoughness, defaultMetallic, 1.0);
    float metallic = metallicRoughness.b;
    float roughness = metallicRoughness.g;
    float ao = hasAO ? texture(pbrMaterial.aoMap, TexCoords).r : defaultAO;
    vec3 emission = hasEmissive ? texture(pbrMaterial.emissiveMap, TexCoords).rgb : vec3(0.0);

    // Get normal
    vec3 N;
    if (hasNormal && useNormalMaps)
    {
        // Sample normal map and transform to world space
        vec3 normalMap = texture(pbrMaterial.normalMap, TexCoords).rgb;
        normalMap = normalMap * 2.0 - 1.0; // Transform from [0,1] to [-1,1]

        N = normalize(TBN * normalMap); // Transform to world space
    } else {
        N = normalize(Normal);
    }

    // View direction
    vec3 V = normalize(camPos - WorldPos);
    float NdotV = max(dot(N, V), 0.0);

    // Calculate fresnel reflectance at normal incidence
    // If dialectric (like plastic) use F0 of 0.04
    // If metal, use albedo color as F0
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Calculate shadow
    float shadow = shadowCalculation(FragPosLightSpace);

    // Initialze reflectance
    vec3 Lo = vec3(0.0);

    // Directional Light
    if (enableDirLight) 
    {
        vec3 dirLightContribution = CalcDirLight(dirLight, N, V, albedo, metallic, roughness, F0);
        dirLightContribution *= (1.0 - shadow); // Apply shadow to directional light
        Lo += dirLightContribution;
    }

    // Point Lights
    for (int i = 0; i < NR_POINT_LIGHTS; ++i)
    {
        Lo += CalcPointLight(pointLights[i], N, WorldPos, V, albedo, metallic, roughness, F0);
    }

    // Spot Light
    if (enableSpotLight) {
        // Transform spot light position to tangent space in the function
        Lo += CalcSpotLight(spotLight, N, WorldPos, V, albedo, metallic, roughness, F0);    
    }

    // Default ambient term if not using IBL
    vec3 ambient = vec3(0.03) * albedo * ao;

    if (useIBL)
    {
        // Sample both the diffuse and specular parts of the IBL
        
        // 1. Diffuse Irradiance (enivronment lighting)
        vec3 F = fresnelSchlickRougness(NdotV, F0, roughness);
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - metallic; 

        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse = irradiance * albedo;

        // 2. Specular reflectance with environment map
        vec3 R = reflect(-V, N);
        // Use rougness to determine the LOD of the prefilterMap
        vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;

        // Get the scale and bias terms from the BRDF LUT
        vec2 brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

        // Combine diffuse and specular IBL contributions
        ambient = (kD * diffuse + specular) * ao;
    }

    // Combine ambient and reflectance and emission
    vec3 color = ambient + Lo + emission;

    // HDR Tonemapping
    color = color / (color + vec3(1.0));

    // Apply exposure adjustment
    // float exposure = 1.0; // Adjust this value as needed (higher = brighter)
    // color = vec3(1.0) - exp(-color * exposure);

    // Gamma correction
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}

vec3 fresnelSchlickRougness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0001); // Prevent division by 0
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, 0.0001); // Prevent division by 0;
}

// Smith's method for combing geometry terms
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{  
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);   
}

float shadowCalculation(vec4 FragPosLightSpace)
{
    // Perform perspective divide
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // calculate bias (based on depth map resolution and slope) on world space normals
    float bias = max(0.05 * (1.0 - dot(normalize(Normal), normalize(-dirLight.direction))), 0.005);

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

vec3 CalcDirLight(DirLight light, vec3 N, vec3 V, vec3 albedo, float metallic, float roughness, vec3 F0)
{
    // Light direction
    vec3 L = normalize(-light.direction);

    // Half vector
    vec3 H = normalize(V + L);

    // Calculate radiance (no attenuation for directional lights)
    vec3 radiance = light.color;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    // Calculate specular component
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // Energy conservation
    vec3 ks = F;
    vec3 kD = vec3(1.0) - ks;
    kD *= 1.0 - metallic; // Metals have no diffuse reflections

    // Scale by NdotL
    float NdotL = max(dot(N, L), 0.0);
    
    // Combine diffuse and specular
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 CalcPointLight(PointLight light, vec3 N, vec3 fragPos, vec3 V, vec3 albedo, float metallic, float roughness, vec3 F0)
{
    // Light direction
    vec3 L = normalize(light.position - fragPos);

    // Half vector
    vec3 H = normalize(V + L);

    // Caclulate distance and attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / max(distance * distance, 0.001);
    vec3 radiance = light.color * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    // Calculate specular component
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // kS is equal to Fresnel
    vec3 ks = F;

    // For energy conservation, the diffuse and specular light can't be above 1.0 (unless the surface is emissive)
    // So we set the diffuse component kD to 1.0 - kS
    vec3 kD = vec3(1.0) - ks;

    // Multiply kD by the inverse metalness such that only non-metals have diffuse lighting,
    // or a linear blend if partially metal (pure metals have no diffuse light)
    kD *= 1.0 - metallic;

    // Scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);

    // outgoing radiance
    return (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
}

vec3 CalcSpotLight(SpotLight light, vec3 N, vec3 fragPos, vec3 V, vec3 albedo, float metallic, float roughness, vec3 F0)
{
    // Light direction
    vec3 L = normalize(light.position - fragPos);

    // Half vector
    vec3 H = normalize(V + L);

    // spotlight intensity
    float theta = dot(L, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // Calculate distance and attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / max(distance * distance, 0.001);
    vec3 radiance = light.color * attenuation * intensity;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    // Calculate specular component
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // Energy conservation
    vec3 ks = F;
    vec3 kD = vec3(1.0) - ks;
    kD *= 1.0 - metallic; // Metals have no diffuse reflections

    // Scale by NdotL
    float NdotL = max(dot(N, L), 0.0);

    // Combine diffuse and specular
    return (kD * albedo / PI + specular) * radiance * NdotL;
}