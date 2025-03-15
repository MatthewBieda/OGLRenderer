#include <unordered_map>
#include <memory>

#include "model.hpp"

struct ModelManager
{
	std::unordered_map<std::string, std::unique_ptr<Model>> models;
	std::string currentModelName;
	std::vector<std::string> modelPaths;

	ModelManager();

	void initialize();

	bool loadModel(const std::string& path);

	Model* getCurrentModel();

	std::vector<std::string> getAvailableModels();

	std::string getCurrentModelName() const;

	void setCurrentModel(const std::string& name);

	void addModelPath(const std::string& path);

	std::string extractModelName(const std::string& path);

};