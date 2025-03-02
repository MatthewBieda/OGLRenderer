#include "include/shader.hpp"
#include <glad/glad.h>

#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	// 1: Retrieve the shader code from file path
	std::string vertexCode;
	std::string fragmentCode;

	try
	{
		vertexCode = readShaderFile(vertexPath);
		fragmentCode = readShaderFile(fragmentPath);
	}
	catch (const std::runtime_error& e) 
	{
		std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ\n" << e.what() << std::endl;
		return;
	}
	
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	// 2. Compile shaders
	uint32_t vertex = compileShader(GL_VERTEX_SHADER, vShaderCode);
	uint32_t fragment = compileShader(GL_FRAGMENT_SHADER, fShaderCode);

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	checkCompilationErrors(ID, "PROGRAM");

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

Shader::~Shader() 
{
	glDeleteProgram(ID);
}

void Shader::use() const
{
	glUseProgram(ID);
}

void Shader::setBool(const std::string_view name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.data()), (int)value);
}
void Shader::setInt(const std::string_view name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.data()), value);
}
void Shader::setFloat(const std::string_view name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.data()), value);
}

std::string Shader::readShaderFile(const std::string& path) 
{
	std::ifstream shaderFile(path);
	if (!shaderFile) 
	{
		throw std::runtime_error("Failed to open shader file: " + path);
	}

	std::stringstream shaderStream;
	shaderStream << shaderFile.rdbuf();
	return shaderStream.str();
}

uint32_t Shader::compileShader(uint32_t shaderType, const char* shaderCode) 
{
	uint32_t shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderCode, nullptr);
	glCompileShader(shader);
	checkCompilationErrors(shader, (shaderType == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT");
	return shader;
}

void Shader::checkCompilationErrors(uint32_t shader, const std::string_view type) const
{
	int success;
	char infoLog[1024];

	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
			std::cerr << "ERROR::SHADER_COMPILATION (" << type << ")\n" << infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
			std::cerr << "ERROR::PROGRAM_LINKING (" << type << ")\n" << infoLog << std::endl;
		}
	}
}