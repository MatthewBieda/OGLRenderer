#pragma once
#include <assimp/scene.h>

#include "mesh.hpp"
#include "shader.hpp"

uint32_t TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

struct Model
{
	std::vector<Texture> textures_loaded;
	std::vector<Mesh> meshes;
	std::string directory;
	bool gammaCorrection;

	// Constructor, expects a filepath to a 3D model
	Model(const std::string& path, bool gamma = false);

	// draws the model, and thus all its meshes
	void Draw(Shader& shader);

	void loadModel(const std::string& path);

	void processNode(aiNode* node, const aiScene* scene);

	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType typeName);
};

