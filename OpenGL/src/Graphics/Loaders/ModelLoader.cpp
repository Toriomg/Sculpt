#include <iostream>
#include <fstream>
#include <string> 

#include "ModelLoader.h"

#include "../Geometry/Mesh.h"
#include "../Shading/Material.h"

bool ModelLoader::LoadModel(std::string& filepath, std::vector<std::shared_ptr<Mesh>>& outMeshes, std::vector<std::shared_ptr<Material>>& outMaterials) {
	std::ifstream inputFile(filepath);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open the file." << std::endl;
        return false; // Indicate an error
    }

    std::string line;
    // Read the file line by line
    while (std::getline(inputFile, line)) {
        // Process each line
        std::string firstWord = line.substr(0, line.find(" "));
    }

    inputFile.close();
    return true;
}
