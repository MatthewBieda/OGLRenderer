#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D hdrBuffer;
uniform float exposure;

void main()
{
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;

    // Apply exposure
    hdrColor *= exposure;

    // Perform Reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / 2.2));

    FragColor = vec4(mapped, 1.0);
}