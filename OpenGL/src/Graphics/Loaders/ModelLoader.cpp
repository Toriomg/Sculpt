#include "ModelLoader.h"

#include "../Geometry/Mesh.h"
#include "../Shading/Material.h"

static bool ModelLoader::LoadModel(std::string& filepath, std::vector<std::shared_ptr<Mesh>>& outMeshes, std::vector<std::shared_ptr<Material>>& outMaterials);
