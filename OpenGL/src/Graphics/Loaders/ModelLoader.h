#pragma once
#include <string>
#include <vector>
#include <memory>

class Mesh;
class Material;

static class ModelLoader {
private:
	ModelLoader() = delete; // Prevent instantiation
public:
	static bool LoadModel(std::string& filepath, std::vector<std::shared_ptr<Mesh>>& outMeshes, std::vector<std::shared_ptr<Material>>& outMaterials);
};