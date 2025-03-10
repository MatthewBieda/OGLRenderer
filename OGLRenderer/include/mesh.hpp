#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "shader.hpp"

struct Vertex 
{
	glm::vec3 position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

enum struct TextureType
{
	Diffuse,
	Specular
};

struct Texture
{
	uint32_t id;
	TextureType type;
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