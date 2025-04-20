#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 5) in mat4 aInstanceMatrix; // Instance transform matrix

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
	gl_Position = lightSpaceMatrix * aInstanceMatrix * vec4(aPos, 1.0);
}