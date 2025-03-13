#pragma once
#include <assimp/scene.h>

#include "mesh.hpp"
#include "shader.hpp"

uint32_t TextureFromFile(const std::string& path, const std::string& directory, bool gamma = false);

struct Model
{
	// Constructor, expects a filepath to a 3D model and takes optional gamma correction
	explicit Model(const std::string& path, bool gamma = false);
	Model(Model&& other) noexcept;
	Model& operator=(Model&& other) noexcept;
	~Model() = default; // Use RAII for cleanup

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	// draws the model, and thus all its meshes
	void Draw(Shader& shader) const;

	void loadModel(std::string_view path);

	void processNode(aiNode* node, const aiScene* scene);

	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType typeName);

	std::vector<Texture> textures_loaded;
	std::vector<Mesh> meshes;
	std::string directory;
	bool gammaCorrection = false;
};

