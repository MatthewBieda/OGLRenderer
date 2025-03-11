#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "shader.hpp"

struct Vertex 
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
	glm::vec2 TexCoords;
};

enum struct TextureType
{
	DIFFUSE,
	SPECULAR
};

struct Texture
{
	uint32_t id;
	TextureType type;
	std::string path;
};

struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Texture> textures;

	Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Texture> textures);
	~Mesh();

	void Draw(Shader &shader);

	uint32_t VAO, VBO, EBO;
	
	void setupMesh();
};