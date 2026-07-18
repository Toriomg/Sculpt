// Loads 3D model files via Assimp and returns a Mesh.
#pragma once
#include <memory>
#include <string>

class Mesh;

class MeshLoader {
public:
    std::shared_ptr<Mesh> Load(std::string const& filepath);
};
