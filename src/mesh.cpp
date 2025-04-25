#include <glad/glad.h>

#include "mesh.hpp"

#include <iostream>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Texture> textures)
	: vertices(std::move(vertices)), indices(std::move(indices)), textures(std::move(textures))
{
}

Mesh::Mesh(const Mesh& other)
	: vertices(other.vertices),
	  indices(other.indices),
	  textures(other.textures),
	  VAO(0), VBO(0), EBO(0)
{
}

Mesh::Mesh(Mesh&& other) noexcept
	: vertices(std::move(other.vertices)),
	  indices(std::move(other.indices)),
	  textures(std::move(other.textures)),
	  VAO(other.VAO),
	  VBO(other.VBO),
	  EBO(other.EBO)
{
	other.VAO = other.VBO = other.EBO = 0;
}

Mesh& Mesh::operator=(const Mesh& other)
{
	if (this != &other)
	{
		cleanup();

		vertices = other.vertices;
		indices = other.indices;
		textures = other.textures;
	}
	return *this;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
	if (this != &other) 
	{
		cleanup();
		vertices = std::move(other.vertices);
		indices = std::move(other.indices);
		textures = std::move(other.textures);
		VAO = other.VAO;
		VBO = other.VBO;
		EBO = other.EBO;
		other.VAO = other.VBO = other.EBO = 0;
	}
	return *this;
}

Mesh::~Mesh()
{
	cleanup();
}

void Mesh::DrawInstanced(Shader& shader, int instanceCount) const
{
	// Define fixed texture units for each type
	const uint32_t DIFFUSE_UNIT = 0;
	const uint32_t SPECULAR_UNIT = 1;
	const uint32_t NORMAL_UNIT = 2;

	// Ensure uniform sampler bindings are correct
	shader.setInt("material.texture_diffuse1", DIFFUSE_UNIT);
	shader.setInt("material.texture_specular1", SPECULAR_UNIT);
	shader.setInt("material.texture_normal1", NORMAL_UNIT);

	// Bind textures to their designated units by type
	bool hasDiffuse = false;
	bool hasSpecular = false;
	bool hasNormal = false;

	for (const Texture& texture : textures)
	{
		switch (texture.type)
		{
		case TextureType::DIFFUSE:
			glActiveTexture(GL_TEXTURE0 + DIFFUSE_UNIT);
			glBindTexture(GL_TEXTURE_2D, texture.id);
			hasDiffuse = true;
			break;
		case TextureType::SPECULAR:
			glActiveTexture(GL_TEXTURE0 + SPECULAR_UNIT);
			glBindTexture(GL_TEXTURE_2D, texture.id);
			hasSpecular = true;
			break;
		case TextureType::NORMAL:
			glActiveTexture(GL_TEXTURE0 + NORMAL_UNIT);
			glBindTexture(GL_TEXTURE_2D, texture.id);
			hasNormal = true;
			break;
		}
	}

	shader.setBool("hasDiffuse", hasDiffuse);
	shader.setBool("hasSpecular", hasSpecular);
	shader.setBool("hasNormal", hasNormal);

	// draw mesh
	glBindVertexArray(VAO);
	glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, instanceCount);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}

void Mesh::setupMesh(GLuint instanceVBO)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

	// positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	// --- Instance Matrix Attributes (mat4 = 4 vec4s) ---
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

	for (int i = 0; i < 4; ++i)
	{
		glEnableVertexAttribArray(5 + i); // <--- start from 5, since 0–4 are taken
		glVertexAttribPointer(5 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
		glVertexAttribDivisor(5 + i, 1); // advance per-instance
	}

	glBindVertexArray(0);
}

void Mesh::cleanup()
{
	if (VAO) glDeleteVertexArrays(1, &VAO);
	if (VBO) glDeleteBuffers(1, &VBO);
	if (EBO) glDeleteBuffers(1, &EBO);

	VAO = VBO = EBO = 0;
}