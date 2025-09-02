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
    float s = size / 2.0f; // Use 's' for brevity

    // This is a standard, known-good cube definition for use with OpenGL.
    // Each line is: Position (XYZ), Normal (XYZ), Tex Coords (UV)
    std::vector<float> vertices = {
        // Back face (Z = -s)
        -s, -s, -s,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         s,  s, -s,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         s, -s, -s,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         s,  s, -s,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -s, -s, -s,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        -s,  s, -s,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

        // Front face (Z = s)
        -s, -s,  s,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         s, -s,  s,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         s,  s,  s,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         s,  s,  s,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -s,  s,  s,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -s, -s,  s,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

        // Left face (X = -s)
        -s,  s,  s, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -s,  s, -s, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -s, -s, -s, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -s, -s, -s, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -s, -s,  s, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -s,  s,  s, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        // Right face (X = s)
         s,  s,  s,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         s, -s, -s,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         s,  s, -s,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         s, -s, -s,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         s,  s,  s,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         s, -s,  s,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

         // Bottom face (Y = -s)
         -s, -s, -s,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
          s, -s, -s,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
          s, -s,  s,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
          s, -s,  s,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         -s, -s,  s,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
         -s, -s, -s,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

         // Top face (Y = s)
         -s,  s, -s,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
          s,  s,  s,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
          s,  s, -s,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
          s,  s,  s,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         -s,  s, -s,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         -s,  s,  s,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
    };

    // This cube is defined without an index buffer. We will draw 36 vertices directly.
    // If your Mesh class REQUIRES an index buffer, you can generate a simple one:
    std::vector<uint32_t> indices(36);
    for (uint32_t i = 0; i < 36; i+=3) {
        indices[i] = i;
        indices[i+1] = i+2;
        indices[i+2] = i+1;
    }

    return CreateMeshFromData(vertices.data(), vertices.size() * sizeof(float), indices.data(), indices.size());
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
        0, 2, 1,   2, 0, 3,     // Base
        4, 6, 5,                 // Front
        7, 9, 8,                 // Back
        10, 12, 11,              // Right
        13, 15, 14               // Left
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
        int k1 = i * (sectors + 1);     // beginning of current stack
        int k2 = k1 + sectors + 1;      // beginning of next stack

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding the first and last stacks
            // k1 => current stack
            // k2 => next stack
            //
            //        k1---k1+1
            //        |  /  |
            //        | /   |
            //        k2---k2+1

            // Triangle 1
            if (i != 0) {  
                indices.push_back(k1);
                indices.push_back(k1 + 1);
                indices.push_back(k2);
            }

            // Triangle 2
            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2 + 1);
                indices.push_back(k2);
            }
        }
    }

    return CreateMeshFromData(vertices.data(), vertices.size() * sizeof(float), indices.data(), indices.size());
}

std::shared_ptr<Mesh> Mesh::CreateTorus(float majorRadius, float minorRadius, int majorSegments, int minorSegments)
{
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    const float PI = 3.14159265359f;

    // Generate vertices
    for (int i = 0; i <= minorSegments; ++i) {
        float v = (float)i / minorSegments;
        float minorAngle = v * 2.0f * PI;

        for (int j = 0; j <= majorSegments; ++j) {
            float u = (float)j / majorSegments;
            float majorAngle = u * 2.0f * PI;

            // --- Vertex Position ---
            // Start with a point on the minor circle in the XY plane
            float x0 = minorRadius * cos(minorAngle);
            float y = minorRadius * sin(minorAngle);
            // Add the major radius to push it out
            float x1 = x0 + majorRadius;
            // Rotate this point around the Y-axis (the major circle)
            float x = x1 * cos(majorAngle);
            float z = x1 * sin(majorAngle);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // --- Normal ---
            // The normal points from the center of the tube's cross-section outward.
            // It's the same calculation as the position, but without the major radius offset.
            float nx0 = cos(minorAngle);
            float ny = sin(minorAngle);
            float nx = nx0 * cos(majorAngle);
            float nz = nx0 * sin(majorAngle);

            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);

            // --- Texture Coordinates ---
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    // Generate indices
    for (int i = 0; i < minorSegments; ++i) {
        for (int j = 0; j < majorSegments; ++j) {
            // Get the four corners of the current quad
            // p1--p2
            // | / |
            // p3--p4
            uint32_t p1 = i * (majorSegments + 1) + j;
            uint32_t p2 = p1 + 1;
            uint32_t p3 = (i + 1) * (majorSegments + 1) + j;
            uint32_t p4 = p3 + 1;

            // First triangle
            indices.push_back(p1);
            indices.push_back(p2);
            indices.push_back(p3);

            // Second triangle
            indices.push_back(p2);
            indices.push_back(p4);
            indices.push_back(p3);
        }
    }

    // Use the private helper to create the mesh from the generated data
    return CreateMeshFromData(vertices.data(), vertices.size() * sizeof(float), indices.data(), indices.size());
}

std::shared_ptr<Mesh> Mesh::CreateDodecahedron() {
	//ITS INCORRECT BUT I DONT CARE
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    const float PI = 3.14159265359f;

    // --- VERTEX GENERATION ---
    // A dodecahedron has 20 vertices. We can define them using the golden ratio.
    const float phi = (1.0f + sqrt(5.0f)) * 0.5f; // Golden ratio
    const float inv_phi = 1.0f / phi;            // 1 / phi

    // The raw vertices of a dodecahedron
    std::vector<float> raw_vertices = {
        // (±1, ±1, ±1) - 8 vertices of a cube
         1,  1,  1,
        -1,  1,  1,
        -1, -1,  1,
         1, -1,  1,
         1,  1, -1,
        -1,  1, -1,
        -1, -1, -1,
         1, -1, -1,
         // (0, ±φ, ±1/φ)
          0,  phi,  inv_phi,
          0, -phi,  inv_phi,
          0,  phi, -inv_phi,
          0, -phi, -inv_phi,
          // (±1/φ, 0, ±φ)
           inv_phi,  0,  phi,
          -inv_phi,  0,  phi,
           inv_phi,  0, -phi,
          -inv_phi,  0, -phi,
          // (±φ, ±1/φ, 0)
           phi,  inv_phi,  0,
          -phi,  inv_phi,  0,
           phi, -inv_phi,  0,
          -phi, -inv_phi,  0
    };

    // Process vertices: normalize positions and calculate normals/UVs
    for (size_t i = 0; i < raw_vertices.size(); i += 3) {
        float x = raw_vertices[i];
        float y = raw_vertices[i + 1];
        float z = raw_vertices[i + 2];

        // Normalize the vertex position to make it unit-sized
        float length = sqrt(x * x + y * y + z * z);
        float nx = x / length;
        float ny = y / length;
        float nz = z / length;

        // Position
        vertices.push_back(nx);
        vertices.push_back(ny);
        vertices.push_back(nz);

        // Normal (for a sphere-like shape, it's the same as the normalized position)
        vertices.push_back(nx);
        vertices.push_back(ny);
        vertices.push_back(nz);

        // Texture Coordinates (using spherical projection)
        float u = 0.5f + atan2(nz, nx) / (2.0f * PI);
        float v = 0.5f - asin(ny) / PI;
        vertices.push_back(u);
        vertices.push_back(v);
    }

    const uint32_t final_faces[12][5] = {
        {0, 8, 10, 4, 16},
        {0, 16, 18, 3, 12},
        {0, 12, 13, 1, 8},
        {1, 13, 2, 19, 17},
        {1, 17, 5, 10, 8},
        {2, 13, 12, 3, 9},
        {3, 18, 7, 11, 9},
        {4, 10, 5, 15, 14},
        {4, 14, 6, 18, 16},
        {5, 17, 19, 6, 15},
        {6, 19, 2, 9, 11},
        {7, 14, 15, 6, 11}
    };
    for (int i = 0; i < 12; ++i) {
        uint32_t v0 = final_faces[i][0];
        uint32_t v1 = final_faces[i][1];
        uint32_t v2 = final_faces[i][2];
        uint32_t v3 = final_faces[i][3];
        uint32_t v4 = final_faces[i][4];

        // Triangulate the pentagon using a fan model from the first vertex (v0).
        // Since the face vertices (v0-v4) are in CCW order, this creates CCW triangles.
        indices.push_back(v0);
        indices.push_back(v1);
        indices.push_back(v2);

        indices.push_back(v0);
        indices.push_back(v2);
        indices.push_back(v3);

        indices.push_back(v0);
        indices.push_back(v3);
        indices.push_back(v4);
    }

    return CreateMeshFromData(vertices.data(), vertices.size() * sizeof(float), indices.data(), indices.size());
}