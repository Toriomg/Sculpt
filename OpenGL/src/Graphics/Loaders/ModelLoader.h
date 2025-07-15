#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <memory>

#include "../Geometry/Vertex.h"
#include "../Geometry/Mesh.h"
#include "../Shading/Material.h"

class Mesh;
class Material;

static class ModelLoader {
private:
	ModelLoader() = delete; // Prevent instantiation
	static bool parseFile(std::string& filepath, std::vector<Vec3>& temp_positions, std::vector<Vec2>& temp_tex_coords, std::vector<Vec3>& temp_normals, std::vector<std::string>& face_lines);
public:
	static bool LoadModel(std::string& filepath, std::shared_ptr<Mesh> outMesh);
};