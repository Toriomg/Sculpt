

#include "ModelLoader.h"



struct IndicesFace {
    int vertex = 0;
    int textCord = 0;
    int normal = 0;
};

void parseFile(const std::string& filepath, std::vector<Vec3>& temp_positions, std::vector<Vec2>& temp_tex_coords, std::vector<Vec3>& temp_normals, std::vector<std::string>& face_lines) {
    std::cout << "" << "Parsing model file: " << filepath << std::endl;

    std::ifstream inputFile(filepath);
    std::string line;
    long index = 0;
    // Read the file line by line
    while (std::getline(inputFile, line)) {
        if (!line.empty()) {
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
        }

        if (line.empty() || line[0] == '#') {
            continue; // Skip empty lines and comments
        }

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
    std::cout << "Finished parsing. Found " << temp_positions.size() << " vertices, "
        << temp_normals.size() << " normals, and " << face_lines.size() << " faces." << std::endl;
    inputFile.close();
}

IndicesFace parseVertexDefinition(const std::string& vertex_def) {
    IndicesFace indices;
    std::istringstream iss(vertex_def);
    std::string v_str, t_str, n_str;

    std::getline(iss, v_str, '/');
    std::getline(iss, t_str, '/');
    std::getline(iss, n_str);

    try {
        if (!v_str.empty()) indices.vertex = std::stol(v_str);
        if (!t_str.empty()) indices.textCord = std::stol(t_str);
        if (!n_str.empty()) indices.normal = std::stol(n_str);
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "ERROR [ModelLoader]: Malformed face definition in .obj file: '" << vertex_def << "'. Contains non-numeric characters." << std::endl;
        return {}; // Return default (all 0) to signal an error
    }
    catch (const std::out_of_range& e) {
        std::cerr << "ERROR [ModelLoader]: Index value out of range in face definition: '" << vertex_def << "'." << std::endl;
        return {}; // Return default (all 0) to signal an error
    }

    return indices;
}

std::shared_ptr<Mesh> LoadModel(const std::string& filepath){
    std::ifstream test_file(filepath);
    if (!test_file.is_open()) {
        std::cerr << "ERROR [ModelLoader]: Could not open file: " << filepath << std::endl;
        return nullptr;
    }
    test_file.close();

	// Vector to hold temporary data
    std::vector<Vec3> temp_positions;
    std::vector<Vec2> temp_tex_coords;
    std::vector<Vec3> temp_normals;
    std::vector<std::string> face_lines;

    parseFile(filepath, temp_positions, temp_tex_coords, temp_normals, face_lines);

	// Now process the collected data
    std::vector<Vertex> final_vertices;
    std::vector<unsigned int> final_indices;

    final_vertices.reserve(temp_positions.size());
    final_indices.reserve(temp_positions.size() * 1.2f);

    std::unordered_map<std::string, unsigned int> vertex_cache;

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

                Vertex new_vertex;

                // Convert from 1-based or relative index to 0-based C++ index
                long v_idx = (indices.vertex > 0) ? indices.vertex - 1 : temp_positions.size() + indices.vertex;
                long t_idx = (indices.textCord > 0) ? indices.textCord - 1 : temp_tex_coords.size() + indices.textCord;
                long n_idx = (indices.normal > 0) ? indices.normal - 1 : temp_normals.size() + indices.normal;

                // Bounds checking
                if (v_idx < 0 || v_idx >= temp_positions.size()) {
                    std::cerr << "ERROR [ModelLoader]: Vertex index " << indices.vertex << " is out of bounds." << std::endl;
                    // In a real engine, you might throw an exception here
                    return nullptr; // Indicate error
                }

                new_vertex.pos = temp_positions[v_idx];

                if (indices.textCord != 0) {
                    if (t_idx < 0 || t_idx >= temp_tex_coords.size()) {
                        std::cerr << "ERROR [ModelLoader]: Texture index " << indices.textCord << " is out of bounds." << std::endl; return nullptr;
                    }
                    // Your Vertex class uses Vec3 for texCoord, but .obj provides Vec2. Adapt as needed.
                    new_vertex.texCoord = temp_tex_coords[t_idx];
                }

                if (indices.normal != 0) {
                    if (n_idx < 0 || n_idx >= temp_normals.size()) {
                        std::cerr << "ERROR [ModelLoader]: Normal index " << indices.normal << " is out of bounds." << std::endl; return nullptr;
                    }
                    new_vertex.normal = temp_normals[n_idx];
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

    if (final_vertices.empty() || final_indices.empty()) {
        std::cerr << "WARNING [ModelLoader]: Model loading resulted in an empty mesh for file: " << filepath << std::endl;
    }

    std::cout << "Successfully loaded model: " << filepath << " with " << final_vertices.size() << " vertices and " << final_indices.size() << " indices." << std::endl;

    return std::make_shared<Mesh>(final_vertices.data(),
        static_cast<unsigned int>(final_vertices.size()),
        final_indices.data(),
        static_cast<unsigned int>(final_indices.size())
    );
}
