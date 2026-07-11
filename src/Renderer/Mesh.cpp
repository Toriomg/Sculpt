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

    CORE_LOG_INFO("Square with {0} faces", indices.size() / 3);
    return CreateMeshFromData(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)), indices.data(), static_cast<uint32_t>(indices.size()));
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
    CORE_LOG_INFO("Pyramid with {0} faces", sizeof(indices) / sizeof(uint32_t) / 3);
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
            float u = static_cast<float>(j) / sectors;
            float v = static_cast<float>(i) / stacks;
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
    CORE_LOG_INFO("Sphere with {0} faces", indices.size() / 3);
    return CreateMeshFromData(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)), indices.data(), static_cast<uint32_t>(indices.size()));
}

std::shared_ptr<Mesh> Mesh::CreateTorus(float majorRadius, float minorRadius, int majorSegments, int minorSegments)
{
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    const float PI = 3.14159265359f;

    // Generate vertices
    for (int i = 0; i <= minorSegments; ++i) {
        float v = static_cast<float>(i) / minorSegments;
        float minorAngle = v * 2.0f * PI;

        for (int j = 0; j <= majorSegments; ++j) {
            float u = static_cast<float>(j) / majorSegments;
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
    CORE_LOG_INFO("Torus with {0} faces", indices.size()/3);
    // Use the private helper to create the mesh from the generated data
    return CreateMeshFromData(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)), indices.data(), static_cast<uint32_t>(indices.size()));
}

std::shared_ptr<Mesh> Mesh::CreateDodecahedron(float size) {
    const float phi = (1.0f + std::sqrt(5.0f)) * 0.5f;
    const float invPhi = 1.0f / phi;
    // Scale so the circumradius (distance from center to any vertex) equals `size`.
    // Raw cube-corner vertices (±1,±1,±1) have length sqrt(3) — the max among all 20.
    const float scale = size / std::sqrt(3.0f);

    const float rawVertices[20][3] = {
        {  1,  1,  1 }, { -1,  1,  1 }, { -1, -1,  1 }, {  1, -1,  1 },
        {  1,  1, -1 }, { -1,  1, -1 }, { -1, -1, -1 }, {  1, -1, -1 },
        {  0,  phi,  invPhi }, {  0, -phi,  invPhi }, {  0,  phi, -invPhi }, {  0, -phi, -invPhi },
        {  invPhi,  0,  phi }, { -invPhi,  0,  phi }, {  invPhi,  0, -phi }, { -invPhi,  0, -phi },
        {  phi,  invPhi,  0 }, { -phi,  invPhi,  0 }, {  phi, -invPhi,  0 }, { -phi, -invPhi,  0 }
    };

    const uint32_t faces[12][5] = {
        { 0,  8, 10,  4, 16 },
        { 0, 16, 18,  3, 12 },
        { 0, 12, 13,  1,  8 },
        { 1, 13,  2, 19, 17 },
        { 1, 17,  5, 10,  8 },
        { 2, 13, 12,  3,  9 },
        { 3, 18,  7, 11,  9 },
        { 4, 10,  5, 15, 14 },
        { 4, 14,  7, 18, 16 },
        { 5, 17, 19,  6, 15 },
        { 6, 19,  2,  9, 11 },
        { 7, 14, 15,  6, 11 }
    };

    constexpr int kFaceCount = 12;
    constexpr int kFaceVerts = 5;
    constexpr int kFloatsPerVertex = 8;
    constexpr float kTwoPi = 2.0f * PI_F;

    std::vector<float> vertices;
    vertices.reserve(kFaceCount * kFaceVerts * kFloatsPerVertex);
    std::vector<uint32_t> indices;
    indices.reserve(kFaceCount * 3 * 3);

    for (int f = 0; f < kFaceCount; ++f) {
        // Flat-shaded: compute one outward normal per pentagonal face.
        const uint32_t i0 = faces[f][0], i1 = faces[f][1], i2 = faces[f][2];
        const float e1x = rawVertices[i1][0] - rawVertices[i0][0];
        const float e1y = rawVertices[i1][1] - rawVertices[i0][1];
        const float e1z = rawVertices[i1][2] - rawVertices[i0][2];
        const float e2x = rawVertices[i2][0] - rawVertices[i0][0];
        const float e2y = rawVertices[i2][1] - rawVertices[i0][1];
        const float e2z = rawVertices[i2][2] - rawVertices[i0][2];
        float nx = e1y * e2z - e1z * e2y;
        float ny = e1z * e2x - e1x * e2z;
        float nz = e1x * e2y - e1y * e2x;
        // Flip if it points inward (dodecahedron is convex + centered at origin).
        if (nx * rawVertices[i0][0] + ny * rawVertices[i0][1] + nz * rawVertices[i0][2] < 0.0f) {
            nx = -nx; ny = -ny; nz = -nz;
        }
        const float nlen = std::sqrt(nx * nx + ny * ny + nz * nz);
        nx /= nlen; ny /= nlen; nz /= nlen;

        const uint32_t baseIndex = static_cast<uint32_t>(vertices.size() / kFloatsPerVertex);
        for (int k = 0; k < kFaceVerts; ++k) {
            const uint32_t vi = faces[f][k];
            vertices.push_back(rawVertices[vi][0] * scale);
            vertices.push_back(rawVertices[vi][1] * scale);
            vertices.push_back(rawVertices[vi][2] * scale);

            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);

            const float angle = PI_F * 0.5f + kTwoPi * k / kFaceVerts;
            vertices.push_back(0.5f + 0.5f * std::cos(angle));
            vertices.push_back(0.5f + 0.5f * std::sin(angle));
        }

        // Triangle fan from the first face vertex.
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 3);
        indices.push_back(baseIndex + 4);
    }

    CORE_LOG_INFO("Dodecahedron with {0} faces", indices.size() / 3);
    return CreateMeshFromData(vertices.data(),
        static_cast<uint32_t>(vertices.size() * sizeof(float)),
        indices.data(),
        static_cast<uint32_t>(indices.size()));
}