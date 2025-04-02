#pragma once
#include <assimp/scene.h>

#include <unordered_map>

#include "mesh.hpp"
#include "shader.hpp"

uint32_t TextureFromFile(const std::string& path, const std::string& directory, bool gamma = false);

struct Model
{
	// Constructor, expects a filepath to a 3D model and takes optional gamma correction
	explicit Model(const std::string& path, bool gamma = false, const std::string& modelName = "Model");
	Model(Model&& other) noexcept;
	Model& operator=(Model&& other) noexcept;
	Model(const Model& other);
	Model& operator=(const Model& other);
	~Model() = default; // Use RAII for cleanup

	// draws the model, and thus all its meshes
	void Draw(Shader& shader) const;

	void loadModel(std::string_view path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType typeName);

	// Textures and meshes loaded from the model
	std::vector<Texture> textures_loaded;
	std::vector<Mesh> meshes;
	std::string directory;

	// Model Properties
	bool gammaCorrection = false;
	bool hasTextures = false;
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f); // x,y,z rotation in degrees
	float scale = 1.0f;
	bool visible = true;
	std::string name;

	// Static member initialization for model name counting
	static std::unordered_map<std::string, int> modelNameCount;
};

