#include "Mesh.h"

std::shared_ptr<Mesh> Mesh::CreateMeshFromData(const void* vertices, uint32_t vertexSize, const uint32_t* indices, uint32_t indexCount)
{
    // 1. Create the low-level GPU buffers
    auto vbo = std::make_shared<VertexBuffer>(vertices, vertexSize, true);
    auto ibo = std::make_shared<IndexBuffer>(indices, indexCount);

    // 2. Define the standard layout for all our primitives
    VertexBufferLayout layout;
    layout.Push<float>(3); // Position (vec3)
    layout.Push<float>(3); // Normal (vec3)
    layout.Push<float>(2); // Texture Coordinates (vec2)

    // 3. Create the VAO and link everything together
    auto vao = std::make_shared<VertexArray>();
    vao->AddBufferPtr(vbo, layout);

    // 4. Create and return the final high-level Mesh object
    return std::make_shared<Mesh>(vao, ibo);
}


std::shared_ptr<Mesh> Mesh::CreateCube(float size) {
    float halfSize = size/2.0f;
    float vertices[] = {
        // Z+ (Front Face)
    -halfSize, -halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
     halfSize, -halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
     halfSize,  halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
    -halfSize,  halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,

    // Z- (Back Face)
    -halfSize, -halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
    -halfSize,  halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     halfSize,  halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
     halfSize, -halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

     // Y+ (Top Face)
     -halfSize,  halfSize, -halfSize,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
     -halfSize,  halfSize,  halfSize,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
      halfSize,  halfSize,  halfSize,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
      halfSize,  halfSize, -halfSize,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,

      // Y- (Bottom Face)
      -halfSize, -halfSize, -halfSize,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
       halfSize, -halfSize, -halfSize,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
       halfSize, -halfSize,  halfSize,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
      -halfSize, -halfSize,  halfSize,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,

      // X+ (Right Face)
       halfSize, -halfSize, -halfSize,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
       halfSize,  halfSize, -halfSize,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
       halfSize,  halfSize,  halfSize,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
       halfSize, -halfSize,  halfSize,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

       // X- (Left Face)
       -halfSize, -halfSize, -halfSize, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
       -halfSize, -halfSize,  halfSize, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
       -halfSize,  halfSize,  halfSize, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
       -halfSize,  halfSize, -halfSize, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f
    };

    uint32_t indices[] = {
    0, 1, 2,   2, 3, 0,       // Front
    4, 5, 6,   6, 7, 4,       // Back
    8, 9, 10,  10, 11, 8,      // Top
    12, 13, 14, 14, 15, 12,    // Bottom
    16, 17, 18, 18, 19, 16,    // Right
    20, 21, 22, 22, 23, 20     // Left
    };

    return CreateMeshFromData(vertices, sizeof(vertices), indices, sizeof(indices)/sizeof(uint32_t));
}
// Add this method to Mesh.cpp
std::shared_ptr<Mesh> Mesh::CreatePyramid(float size) {
    float halfSize = size / 2.0f;
    float height = size; // You can make this a separate parameter if you wish

    float vertices[] = {
        // --- Base Face (Y-) ---
        // Positions(3)           Normals(3)             TexCoords(2)
        -halfSize, 0.0f, -halfSize,   0.0f, -1.0f,  0.0f,    0.0f, 1.0f,
         halfSize, 0.0f, -halfSize,   0.0f, -1.0f,  0.0f,    1.0f, 1.0f,
         halfSize, 0.0f,  halfSize,   0.0f, -1.0f,  0.0f,    1.0f, 0.0f,
        -halfSize, 0.0f,  halfSize,   0.0f, -1.0f,  0.0f,    0.0f, 0.0f,

        // --- Front Face (Z+) ---
        -halfSize, 0.0f,  halfSize,   0.0f,  0.447f, 0.894f,  0.0f, 0.0f,
         halfSize, 0.0f,  halfSize,   0.0f,  0.447f, 0.894f,  1.0f, 0.0f,
         0.0f,     height, 0.0f,      0.0f,  0.447f, 0.894f,  0.5f, 1.0f,

         // --- Back Face (Z-) ---
          halfSize, 0.0f, -halfSize,   0.0f,  0.447f, -0.894f, 1.0f, 0.0f,
         -halfSize, 0.0f, -halfSize,   0.0f,  0.447f, -0.894f, 0.0f, 0.0f,
          0.0f,     height, 0.0f,      0.0f,  0.447f, -0.894f, 0.5f, 1.0f,

          // --- Right Face (X+) ---
           halfSize, 0.0f,  halfSize,   0.894f, 0.447f,  0.0f,   1.0f, 0.0f,
           halfSize, 0.0f, -halfSize,   0.894f, 0.447f,  0.0f,   0.0f, 0.0f,
           0.0f,     height, 0.0f,      0.894f, 0.447f,  0.0f,   0.5f, 1.0f,

           // --- Left Face (X-) ---
           -halfSize, 0.0f, -halfSize,  -0.894f, 0.447f,  0.0f,   0.0f, 0.0f,
           -halfSize, 0.0f,  halfSize,  -0.894f, 0.447f,  0.0f,   1.0f, 0.0f,
            0.0f,     height, 0.0f,     -0.894f, 0.447f,  0.0f,   0.5f, 1.0f
    };

    uint32_t indices[] = {
        0, 1, 2,   2, 3, 0,     // Base
        4, 5, 6,                 // Front
        7, 8, 9,                 // Back
        10, 11, 12,              // Right
        13, 14, 15               // Left
    };

    return CreateMeshFromData(vertices, sizeof(vertices), indices, sizeof(indices) / sizeof(uint32_t));
}

// Add this method to Mesh.cpp
std::shared_ptr<Mesh> Mesh::CreateSphere(float radius, int sectors, int stacks) {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    const float PI = 3.14159265359f;
    float sectorStep = 2 * PI / sectors;
    float stackStep = PI / stacks;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = PI / 2 - i * stackStep;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;

            // Vertex position
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normal (for a sphere, it's just the normalized position)
            float nx = x / radius;
            float ny = y / radius;
            float nz = z / radius;
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);

            // Texture coords
            float u = (float)j / sectors;
            float v = (float)i / stacks;
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    return CreateMeshFromData(vertices.data(), vertices.size() * sizeof(float), indices.data(), indices.size());
}