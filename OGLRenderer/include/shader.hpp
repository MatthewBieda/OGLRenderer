#pragma once

#include <string>

struct Shader 
{
	uint32_t ID;

	// constructor reads and builds the shader using utility functions
	Shader(const char* vertexPath, const char* fragmentPath);
	std::string readShaderFile(const std::string& path);
	uint32_t compileShader(uint32_t shaderType, const char* shaderCode);

	// activate shader
	void use() const;

	// utility uniform functions
	void setBool(const std::string_view name, bool value) const;
	void setInt(const std::string_view name, int value) const;
	void setFloat(const std::string_view name, float value) const;

	// error checking
	void checkCompilationErrors(uint32_t shader, const std::string_view type) const;

	~Shader();
};