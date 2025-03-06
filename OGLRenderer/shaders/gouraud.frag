#version 330 core

in vec3 FragColor; // Interpolated from Vertex Shader

out vec4 FinalColor;

void main()
{
	FinalColor = vec4(FragColor, 1.0);
}