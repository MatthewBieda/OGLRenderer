#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "shader.hpp"

struct Vertex 
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
};

enum struct TextureType
{
	ALBEDO,
	NORMAL,
	METALLIC,
	ROUGHNESS,
	AO,
	EMISSIVE
};

struct Texture
{
	uint32_t id;
	TextureType type;
	std::string path;
};

struct Mesh
{
	Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Texture> textures);
	Mesh(Mesh&& other) noexcept;
	Mesh& operator=(Mesh&& other) noexcept;
	~Mesh();

	Mesh(const Mesh& other); // Copy constructor
	Mesh& operator=(const Mesh& other); // Copy assignment operator

	void DrawInstanced(Shader &shader, int instanceCount) const;
	void setupMesh(GLuint instanceVBO);
	void cleanup();

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Texture> textures;

	uint32_t VAO{ 0 }, VBO{ 0 }, EBO{0};
};