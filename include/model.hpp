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
	void Draw(Shader& shader, size_t instanceCount) const;

	void loadModel(std::string_view path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType typeName);

	// Global modal properties
	std::string name;
	std::vector<Texture> textures_loaded;
	std::vector<Mesh> meshes;
	std::string directory;
	bool gammaCorrection = false;
	bool hasTextures = false;
	bool visible = true;

	GLuint instanceVBO = 0;

	// Static member initialization for model name counting
	static std::unordered_map<std::string, int> modelNameCount;
};

