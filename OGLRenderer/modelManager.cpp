#include "modelManager.hpp"
#include <iostream>
#include <filesystem>

ModelManager::ModelManager()
{
    modelPaths = {
        "assets/models/backpack/backpack.obj",
        "assets/models/bunny/bunny.obj"
    };
}

void ModelManager::initialize()
{
    if (!modelPaths.empty())
    {
        loadModel(modelPaths[0]);
    }
}

bool ModelManager::loadModel(const std::string& path)
{
    try
    {
        // Extract model name from path
        std::string name = extractModelName(path);

        // Check if the model is already loaded
        if (models.find(name) == models.end())
        {
            // Create the model 
            models[name] = std::make_unique<Model>(path);
        }

        currentModelName = name;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to load model " << path << ": " << e.what() << std::endl;
        return false;
    }
}

Model* ModelManager::getCurrentModel()
{
    auto it = models.find(currentModelName);
    return (it != models.end()) ? it->second.get() : nullptr;
}

std::vector<std::string> ModelManager::getAvailableModels()
{
    std::vector<std::string> names;
    names.reserve(modelPaths.size());

    for (const std::string& path : modelPaths)
    {
        names.push_back(extractModelName(path));
    }
    return names;
}

std::string ModelManager::getCurrentModelName() const
{
    return currentModelName;
}

void ModelManager::setCurrentModel(const std::string& name)
{
    // Find the exact path that matches this model name
    for (const std::string& path : modelPaths)
    {
        if (extractModelName(path) == name)
        {
            loadModel(path);
            return;
        }
    }

    std::cerr << "Model name not found: " << name << std::endl;
}

void ModelManager::addModelPath(const std::string& path)
{
    modelPaths.push_back(path);
}

std::string ModelManager::extractModelName(const std::string& path)
{
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash == std::string::npos) {
        lastSlash = 0;
    }
    else {
        lastSlash += 1;
    }

    size_t lastDot = path.find_last_of('.');
    if (lastDot == std::string::npos || lastDot < lastSlash) {
        return path.substr(lastSlash);
    }

    return path.substr(lastSlash, lastDot - lastSlash);
}