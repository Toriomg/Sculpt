

#include "ModelLoader.h"



struct IndicesFace {
    unsigned int vertex = 0;
    unsigned int textCord = 0;
    unsigned int normal = 0;
};

void parseFile(const std::string& filepath, std::vector<Vec3>& temp_positions, std::vector<Vec2>& temp_tex_coords, std::vector<Vec3>& temp_normals, std::vector<std::string>& face_lines) {
    std::ifstream inputFile(filepath);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open the file." << std::endl;
        inputFile.close();
    }
    std::string line;
    // Read the file line by line
    while (std::getline(inputFile, line)) {
        // Process each line
        std::string firstWord = line.substr(0, line.find(" "));
        if (firstWord == "v") {
            // Vertex position
            Vec3 position;
            std::istringstream iss(line.substr(2));
            iss >> position.x >> position.y >> position.z;
            temp_positions.push_back(position);
        }
        else if (firstWord == "vt") {
            // Texture coordinate
            Vec2 texCoord;
            std::istringstream iss(line.substr(3));
            iss >> texCoord.x >> texCoord.y;
            temp_tex_coords.push_back(texCoord);
        }
        else if (firstWord == "vn") {
            // Vertex normal
            Vec3 normal;
            std::istringstream iss(line.substr(3));
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (firstWord == "f") {
            // Face definition
            face_lines.push_back(line.substr(2)); // Store the face line without the 'f' prefix
        }
    }
    inputFile.close();
}

IndicesFace parseVertexDefinition(const std::string& vertex_def) {
    IndicesFace indices;
    std::istringstream iss(vertex_def);
    std::string v_str, t_str, n_str;

    std::getline(iss, v_str, '/');
    std::getline(iss, t_str, '/');
    std::getline(iss, n_str);

    if (!v_str.empty()) indices.vertex = std::stoi(v_str);
    if (!t_str.empty()) indices.textCord = std::stoi(t_str);
    if (!n_str.empty()) indices.normal = std::stoi(n_str);

    return indices;
}

std::shared_ptr<Mesh> LoadModel(const std::string& filepath){
	// Vector to hold temporary data
    std::vector<Vec3> temp_positions;
    std::vector<Vec2> temp_tex_coords;
    std::vector<Vec3> temp_normals;
    std::vector<std::string> face_lines;

    parseFile(filepath, temp_positions, temp_tex_coords, temp_normals, face_lines);

	// Now process the collected data
    std::vector<Vertex> final_vertices;
    std::vector<unsigned int> final_indices;
    std::map<std::string, unsigned int> vertex_cache;

    for (const auto& face_line : face_lines) {
        std::istringstream ss(face_line);
        std::string vertex_def;

        while (ss >> vertex_def) {
            if (vertex_cache.find(vertex_def) != vertex_cache.end()) {
                // CACHE HIT
                unsigned int existing_index = vertex_cache[vertex_def];
                final_indices.push_back(existing_index);
            }
            else {
				// CACHE MISS
                IndicesFace indices = parseVertexDefinition(vertex_def);

                // Los índices en .obj son 1-based, los de C++ son 0-based
                Vertex new_vertex;
                if (indices.vertex > 0) {
                    new_vertex.pos = temp_positions[indices.vertex - 1];
                }
                if (indices.textCord > 0) {
                    new_vertex.texCoord = temp_tex_coords[indices.textCord - 1];
                }
                if (indices.normal > 0) {
                    new_vertex.normal = temp_normals[indices.normal - 1];
                }

                // Add to the buffer
                final_vertices.push_back(new_vertex);

                unsigned int new_index = final_vertices.size() - 1;
                final_indices.push_back(new_index);

                // Save in cache
                vertex_cache[vertex_def] = new_index;
            }
        }
    }

    return std::make_shared<Mesh>(final_vertices.data(),
        static_cast<unsigned int>(final_vertices.size() * sizeof(Vertex)),
        final_indices.data(),
        static_cast<unsigned int>(final_indices.size())
    );
}
