#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <memory>

class Vertex;
class Mesh;
class Material;

static class ModelLoader {
private:
	ModelLoader() = delete; // Prevent instantiation
	static void parseFile(std::string& filepath, std::vector<Vec3>& temp_positions, std::vector<Vec2>& temp_tex_coords, std::vector<Vec3>& temp_normals, std::vector<std::string>& face_lines);
public:
	static Mesh LoadModel(std::string& filepath);
};