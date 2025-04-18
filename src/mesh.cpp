#include <glad/glad.h>

#include "mesh.hpp"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Texture> textures)
	: vertices(std::move(vertices)), indices(std::move(indices)), textures(std::move(textures))
{
	setupMesh();
}

Mesh::Mesh(const Mesh& other)
	: vertices(other.vertices),
	  indices(other.indices),
	  textures(other.textures),
	  VAO(0), VBO(0), EBO(0)
{
	setupMesh(); // Create new buffers for this copy
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

		setupMesh();
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

void Mesh::Draw(Shader& shader) const
{
	uint32_t diffuseNr = 1;
	uint32_t specularNr = 1;
	uint32_t normalNr = 1;
	uint32_t heightNr = 1;

	for (size_t i = 0; i < textures.size(); ++i) 
	{
		glActiveTexture(GL_TEXTURE0 + i); // activate correct texture unit

		std::string uniformName;
		if (textures[i].type == TextureType::DIFFUSE) 
		{
			uniformName = "material.texture_diffuse" + std::to_string(diffuseNr++);
		}
		else if (textures[i].type == TextureType::SPECULAR) 
		{
			uniformName = "material.texture_specular" + std::to_string(specularNr++);
		}
		else if (textures[i].type == TextureType::NORMAL)
		{
			uniformName = "material.texture_normal" + std::to_string(normalNr++);
		}
		else if (textures[i].type == TextureType::HEIGHT)
		{
			uniformName = "material.texture_height" + std::to_string(heightNr++);
		}

		glUniform1i(glGetUniformLocation(shader.ID, uniformName.c_str()), i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}

	// draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}

void Mesh::setupMesh()
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

	glBindVertexArray(0);
}

void Mesh::cleanup()
{
	if (VAO) glDeleteVertexArrays(1, &VAO);
	if (VBO) glDeleteBuffers(1, &VBO);
	if (EBO) glDeleteBuffers(1, &EBO);

	VAO = VBO = EBO = 0;
}