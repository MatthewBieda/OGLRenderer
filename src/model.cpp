#include <glad/glad.h>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>

#include "mesh.hpp"
#include "model.hpp"

std::unordered_map<std::string, int> Model::modelNameCount;

Model::Model(const std::string& path, bool gamma, const std::string& modelName)
	: gammaCorrection(gamma)
{
	name = modelName + std::to_string(++modelNameCount[modelName]);
	loadModel(path);
}

Model::Model(const Model& other)
    : meshes(other.meshes),
	  directory(other.directory),
	  textures_loaded(other.textures_loaded),
	  gammaCorrection(other.gammaCorrection),
	  hasTextures(other.hasTextures),
	  visible(other.visible),
	  name(other.name + std::to_string(++modelNameCount[other.name]))
{
	std::cout << "Copying model with name: " << other.name << " to " << name << std::endl;
}

Model& Model::operator=(const Model& other)
{
	if (this != &other) {
		meshes = other.meshes;
		directory = other.directory;
		textures_loaded = other.textures_loaded;
		gammaCorrection = other.gammaCorrection;
		hasTextures = other.hasTextures;
		visible = other.visible;
		name = other.name + std::to_string(++modelNameCount[other.name]);
	}
	return *this;
}

Model::Model(Model&& other) noexcept
    : meshes(std::move(other.meshes)),
	  directory(std::move(other.directory)),
	  textures_loaded(std::move(other.textures_loaded)),
	  gammaCorrection(other.gammaCorrection),
	  hasTextures(other.hasTextures),
	  visible(other.visible),
	  name(std::move(other.name))
{
}

Model& Model::operator=(Model&& other) noexcept
{
	if (this != &other) {
		meshes = std::move(other.meshes);
		directory = std::move(other.directory);
		textures_loaded = std::move(other.textures_loaded);
		gammaCorrection = other.gammaCorrection;
		hasTextures = other.hasTextures;
		visible = other.visible;
		name = std::move(other.name);
	}
	return *this;
}

void Model::Draw(Shader& shader, size_t instanceCount) const
{
	// Set uniform before drawing meshes
	glUniform1i(glGetUniformLocation(shader.ID, "hasTextures"), hasTextures);

	for (const auto& mesh : meshes)
	{
		mesh.DrawInstanced(shader, instanceCount);
	}
}

void Model::loadModel(std::string_view path)
{
	// Read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		std::string(path), 
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_SortByPType |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_OptimizeMeshes
	);

	// Check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}
	
	// Retrieve the directory path of the filepath
	std::string pathStr(path);
	directory = pathStr.substr(0, path.find_last_of('/'));

	// Process Assimp's root node recursively
	processNode(scene->mRootNode, scene);

	// Create and upload instance VBO
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glm::mat4 identity(1.0f);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4), &identity, GL_DYNAMIC_DRAW);

	for (Mesh& mesh : meshes)
	{
		mesh.setupMesh(instanceVBO);
	}
}

void Model::processNode(aiNode* node, const aiScene *scene)
{
	// Process each mesh located at the current node
	meshes.reserve(meshes.size() + node->mNumMeshes);
	for (uint32_t i = 0; i < node->mNumMeshes; ++i)
	{
		// The node object only contains indices to index the actual objects in the scene.
		// The scene object contains all the data, node is just to keep stuff organized (like relations between nodes)
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// After we've processed all of the meshes (if any) we then recursively proecss each of the children nodes
	for (uint32_t i = 0; i < node->mNumChildren; ++i)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	// Data to fill
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Texture> textures;

	vertices.reserve(mesh->mNumVertices);

	// Walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		Vertex vertex;

		// Position
		vertex.Position = {
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z
		};

		// Normals
		if (mesh->HasNormals()) {
			vertex.Normal = {
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z
			};
		}

		// Texture Coordinates
		if (mesh->mTextureCoords[0]) {
			vertex.TexCoords = {
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			};

			// Tangent
			vertex.Tangent = {
				mesh->mTangents[i].x,
				mesh->mTangents[i].y,
				mesh->mTangents[i].z
			};

			// Bitangent
			vertex.Bitangent = {
				mesh->mBitangents[i].x,
				mesh->mBitangents[i].y,
				mesh->mBitangents[i].z
			};
		}
		else {
			vertex.TexCoords = { 0.0f, 0.0f };
		}

		vertices.push_back(vertex);
	}

	// Reserve memory for indices
	uint32_t totalIndices = 0;
	for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
	{
		totalIndices += mesh->mFaces[i].mNumIndices;
	}
	indices.reserve(totalIndices);

	// Process indices
	for (uint32_t i = 0; i < mesh->mNumFaces; ++i) 
	{
		aiFace face = mesh->mFaces[i];

		// Retrieve all indices of the face and store them in the indicies vector
		for (uint32_t j = 0; j < face.mNumIndices; ++j) 
		{
			indices.push_back(face.mIndices[j]);
		}
	}
	
	// Process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	// We assume a convention for sampler name in the shaders. 
	// Each diffuse texture should be named as "texture_diffuseN" wher N ranges from 1 through MAX_SAMPLER_NUMBER
	// The same applies to the other texture types, i.e.
	// diffuse: texture_diffuseN
	// specular: texture_specularN
	// normal: texture_normalN

	// 1: Diffuse Maps
	std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::DIFFUSE);
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

	// 2: Specular Maps
	std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::SPECULAR);
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

	// 3: Normal/Height Maps
	std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, TextureType::NORMAL);
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

	if (!diffuseMaps.empty())
	{
		hasTextures = true;
	}

	return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType typeName)
{
	std::vector<Texture> textures;
	textures.reserve(mat->GetTextureCount(type));

	for (uint32_t i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		// Check if the texture was already loaded, if so continue to next iteration
		bool skip = false;
		for (const auto& loaded_texture: textures_loaded)
		{
			if (std::strcmp(loaded_texture.path.data(), str.C_Str()) == 0)
			{
				textures.push_back(loaded_texture);
				skip = true;
				break;
			}
		}

		if (!skip)
		{
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), directory, gammaCorrection);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture); 
		}
	}

	return textures;
}

uint32_t TextureFromFile(const std::string& path, const std::string& directory, bool gamma)
{
	uint32_t textureID;
	glGenTextures(1, &textureID);

	std::string fullPath = directory + '/' + path;

	int width, height, nrChannels;
	uint8_t* data = stbi_load(fullPath.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format = GL_RGB;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0,
			gamma ? (format == GL_RGBA ? GL_SRGB_ALPHA : GL_SRGB) : format,
			width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Texture wrapping/filtering options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cerr << "Failed to load texture" << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}