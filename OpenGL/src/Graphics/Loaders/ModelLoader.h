#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <string_view>
#include <charconv>
#include <array>

#include "../Geometry/Vertex.h"
#include "../Geometry/Mesh.h"
#include "../Shading/Material.h"


std::shared_ptr<Mesh> LoadModel(const std::string& filepath);