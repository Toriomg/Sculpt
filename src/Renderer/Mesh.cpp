#include "Mesh.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"
#include "Platform/Graphics/Vertex.hpp"
#include <cmath>
#include <numbers>
#include <vector>

std::shared_ptr<Mesh> Mesh::CreateMeshFromData(void const* vertices, uint32_t vertexSize,
                                               uint32_t const* indices, uint32_t indexCount) {
    // 1. Create the low-level GPU buffers
    auto vbo = std::make_shared<VertexBuffer>(vertices, vertexSize, true);
    auto ibo = std::make_shared<IndexBuffer>(indices, indexCount);

    // 2. Define the standard layout for all our primitives
    VertexBufferLayout layout;
    layout.Push<float>(3);  // Position (vec3)
    layout.Push<float>(3);  // Normal (vec3)
    layout.Push<float>(2);  // Texture Coordinates (vec2)

    // 3. Create the VAO and link everything together
    auto vao = std::make_shared<VertexArray>();
    vao->AddBufferPtr(vbo, layout);

    // 4. Create and return the final high-level Mesh object
    uint32_t const vertexCount = vertexSize / static_cast<uint32_t>(sizeof(Vertex));
    return std::make_shared<Mesh>(vao, ibo, vertexCount);
}

std::shared_ptr<Mesh> Mesh::CreateCube(float size) {
    float s = size / 2.0f;  // Use 's' for brevity

    // This is a standard, known-good cube definition for use with OpenGL.
    // Each line is: Position (XYZ), Normal (XYZ), Tex Coords (UV)
    std::vector<float> vertices = {
      // Back face (Z = -s)
      -s, -s, -s, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, s, s, -s, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, s, -s, -s,
      0.0f, 0.0f, -1.0f, 1.0f, 0.0f, s, s, -s, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, -s, -s, -s, 0.0f,
      0.0f, -1.0f, 0.0f, 0.0f, -s, s, -s, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,

      // Front face (Z = s)
      -s, -s, s, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, s, -s, s, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, s, s, s,
      0.0f, 0.0f, 1.0f, 1.0f, 1.0f, s, s, s, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, -s, s, s, 0.0f, 0.0f,
      1.0f, 0.0f, 1.0f, -s, -s, s, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

      // Left face (X = -s)
      -s, s, s, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, -s, s, -s, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, -s, -s, -s,
      -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, -s, -s, -s, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, -s, -s, s, -1.0f,
      0.0f, 0.0f, 0.0f, 0.0f, -s, s, s, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

      // Right face (X = s)
      s, s, s, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, s, -s, -s, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, s, s, -s,
      1.0f, 0.0f, 0.0f, 1.0f, 1.0f, s, -s, -s, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, s, s, s, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f, s, -s, s, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

      // Bottom face (Y = -s)
      -s, -s, -s, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, s, -s, -s, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, s, -s, s,
      0.0f, -1.0f, 0.0f, 1.0f, 0.0f, s, -s, s, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, -s, -s, s, 0.0f,
      -1.0f, 0.0f, 0.0f, 0.0f, -s, -s, -s, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

      // Top face (Y = s)
      -s, s, -s, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, s, s, s, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, s, s, -s,
      0.0f, 1.0f, 0.0f, 1.0f, 1.0f, s, s, s, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, -s, s, -s, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f, -s, s, s, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f};

    // The cube uses 36 unique vertices (no shared corners across faces, for correct flat normals),
    // so a sequential IBO adds no benefit — it exists only because Mesh always requires an IBO.
    std::vector<uint32_t> indices(36);
    for (uint32_t i = 0; i < 36; i += 3) {
        indices[i]     = i;
        indices[i + 1] = i + 2;
        indices[i + 2] = i + 1;
    }

    CORE_LOG_INFO("Square with {0} faces", indices.size() / 3);
    return CreateMeshFromData(vertices.data(),
                              static_cast<uint32_t>(vertices.size() * sizeof(float)),
                              indices.data(), static_cast<uint32_t>(indices.size()));
}
// Add this method to Mesh.cpp
std::shared_ptr<Mesh> Mesh::CreatePyramid(float size) {
    float halfSize = size / 2.0f;
    float height   = size;  // You can make this a separate parameter if you wish

    float vertices[] = {
      // --- Base Face (Y-) ---
      // Positions(3)           Normals(3)             TexCoords(2)
      -halfSize, 0.0f, -halfSize, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, halfSize, 0.0f, -halfSize, 0.0f,
      -1.0f, 0.0f, 1.0f, 1.0f, halfSize, 0.0f, halfSize, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, -halfSize,
      0.0f, halfSize, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,

      // --- Front Face (Z+) ---
      -halfSize, 0.0f, halfSize, 0.0f, 0.447f, 0.894f, 0.0f, 0.0f, halfSize, 0.0f, halfSize, 0.0f,
      0.447f, 0.894f, 1.0f, 0.0f, 0.0f, height, 0.0f, 0.0f, 0.447f, 0.894f, 0.5f, 1.0f,

      // --- Back Face (Z-) ---
      halfSize, 0.0f, -halfSize, 0.0f, 0.447f, -0.894f, 1.0f, 0.0f, -halfSize, 0.0f, -halfSize,
      0.0f, 0.447f, -0.894f, 0.0f, 0.0f, 0.0f, height, 0.0f, 0.0f, 0.447f, -0.894f, 0.5f, 1.0f,

      // --- Right Face (X+) ---
      halfSize, 0.0f, halfSize, 0.894f, 0.447f, 0.0f, 1.0f, 0.0f, halfSize, 0.0f, -halfSize, 0.894f,
      0.447f, 0.0f, 0.0f, 0.0f, 0.0f, height, 0.0f, 0.894f, 0.447f, 0.0f, 0.5f, 1.0f,

      // --- Left Face (X-) ---
      -halfSize, 0.0f, -halfSize, -0.894f, 0.447f, 0.0f, 0.0f, 0.0f, -halfSize, 0.0f, halfSize,
      -0.894f, 0.447f, 0.0f, 1.0f, 0.0f, 0.0f, height, 0.0f, -0.894f, 0.447f, 0.0f, 0.5f, 1.0f};

    uint32_t indices[] = {
      0,  2,  1,  2, 0, 3,  // Base
      4,  6,  5,            // Front
      7,  9,  8,            // Back
      10, 12, 11,           // Right
      13, 15, 14            // Left
    };
    CORE_LOG_INFO("Pyramid with {0} faces", sizeof(indices) / sizeof(uint32_t) / 3);
    return CreateMeshFromData(vertices, sizeof(vertices), indices,
                              sizeof(indices) / sizeof(uint32_t));
}

// Add this method to Mesh.cpp
std::shared_ptr<Mesh> Mesh::CreateSphere(float radius, int sectors, int stacks) {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    float const PI   = 3.14159265359f;
    float sectorStep = 2 * PI / sectors;
    float stackStep  = PI / stacks;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = PI / 2 - i * stackStep;
        float xy         = radius * cosf(stackAngle);
        float z          = radius * sinf(stackAngle);

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
        int k1 = i * (sectors + 1);  // beginning of current stack
        int k2 = k1 + sectors + 1;   // beginning of next stack

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            // Standard UV-sphere quad triangulation:
            //        k1---k1+1
            //        |  /  |
            //        k2---k2+1
            //
            // The top cap (i==0) skips Triangle 1 and the bottom cap (i==stacks-1) skips
            // Triangle 2 because those quads degenerate to triangles at the poles.
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k1 + 1);
                indices.push_back(k2);
            }

            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2 + 1);
                indices.push_back(k2);
            }
        }
    }
    CORE_LOG_INFO("Sphere with {0} faces", indices.size() / 3);
    return CreateMeshFromData(vertices.data(),
                              static_cast<uint32_t>(vertices.size() * sizeof(float)),
                              indices.data(), static_cast<uint32_t>(indices.size()));
}

std::shared_ptr<Mesh> Mesh::CreateTorus(float majorRadius, float minorRadius, int majorSegments,
                                        int minorSegments) {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    float const PI = 3.14159265359f;

    // Generate vertices
    for (int i = 0; i <= minorSegments; ++i) {
        float v          = static_cast<float>(i) / minorSegments;
        float minorAngle = v * 2.0f * PI;

        for (int j = 0; j <= majorSegments; ++j) {
            float u          = static_cast<float>(j) / majorSegments;
            float majorAngle = u * 2.0f * PI;

            // --- Vertex Position ---
            // Start with a point on the minor circle in the XY plane
            float x0 = minorRadius * cos(minorAngle);
            float y  = minorRadius * sin(minorAngle);
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
            float ny  = sin(minorAngle);
            float nx  = nx0 * cos(majorAngle);
            float nz  = nx0 * sin(majorAngle);

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
    CORE_LOG_INFO("Torus with {0} faces", indices.size() / 3);
    // Use the private helper to create the mesh from the generated data
    return CreateMeshFromData(vertices.data(),
                              static_cast<uint32_t>(vertices.size() * sizeof(float)),
                              indices.data(), static_cast<uint32_t>(indices.size()));
}

std::shared_ptr<Mesh> Mesh::CreateDodecahedron(float size) {
    float const phi    = (1.0f + std::sqrt(5.0f)) * 0.5f;
    float const invPhi = 1.0f / phi;
    // Scale so the circumradius (distance from center to any vertex) equals `size`.
    // Raw cube-corner vertices (±1,±1,±1) have length sqrt(3) — the max among all 20.
    float const scale = size / std::sqrt(3.0f);

    float const rawVertices[20][3] = {
      {      1,       1,       1},
      {     -1,       1,       1},
      {     -1,      -1,       1},
      {      1,      -1,       1},
      {      1,       1,      -1},
      {     -1,       1,      -1},
      {     -1,      -1,      -1},
      {      1,      -1,      -1},
      {      0,     phi,  invPhi},
      {      0,    -phi,  invPhi},
      {      0,     phi, -invPhi},
      {      0,    -phi, -invPhi},
      { invPhi,       0,     phi},
      {-invPhi,       0,     phi},
      { invPhi,       0,    -phi},
      {-invPhi,       0,    -phi},
      {    phi,  invPhi,       0},
      {   -phi,  invPhi,       0},
      {    phi, -invPhi,       0},
      {   -phi, -invPhi,       0}
    };

    uint32_t const faces[12][5] = {
      {0,  8, 10,  4, 16},
      {0, 16, 18,  3, 12},
      {0, 12, 13,  1,  8},
      {1, 13,  2, 19, 17},
      {1, 17,  5, 10,  8},
      {2, 13, 12,  3,  9},
      {3, 18,  7, 11,  9},
      {4, 10,  5, 15, 14},
      {4, 14,  7, 18, 16},
      {5, 17, 19,  6, 15},
      {6, 19,  2,  9, 11},
      {7, 14, 15,  6, 11}
    };

    constexpr int kFaceCount       = 12;
    constexpr int kFaceVerts       = 5;
    constexpr int kFloatsPerVertex = 8;
    constexpr float kTwoPi         = 2.0f * PI_F;

    std::vector<float> vertices;
    vertices.reserve(kFaceCount * kFaceVerts * kFloatsPerVertex);
    std::vector<uint32_t> indices;
    indices.reserve(kFaceCount * 3 * 3);

    for (int f = 0; f < kFaceCount; ++f) {
        // Flat-shaded: compute one outward normal per pentagonal face.
        uint32_t const i0 = faces[f][0], i1 = faces[f][1], i2 = faces[f][2];
        float const e1x = rawVertices[i1][0] - rawVertices[i0][0];
        float const e1y = rawVertices[i1][1] - rawVertices[i0][1];
        float const e1z = rawVertices[i1][2] - rawVertices[i0][2];
        float const e2x = rawVertices[i2][0] - rawVertices[i0][0];
        float const e2y = rawVertices[i2][1] - rawVertices[i0][1];
        float const e2z = rawVertices[i2][2] - rawVertices[i0][2];
        float nx        = e1y * e2z - e1z * e2y;
        float ny        = e1z * e2x - e1x * e2z;
        float nz        = e1x * e2y - e1y * e2x;
        // Flip if it points inward (dodecahedron is convex + centered at origin).
        if (nx * rawVertices[i0][0] + ny * rawVertices[i0][1] + nz * rawVertices[i0][2] < 0.0f) {
            nx = -nx;
            ny = -ny;
            nz = -nz;
        }
        float const nlen = std::sqrt(nx * nx + ny * ny + nz * nz);
        nx /= nlen;
        ny /= nlen;
        nz /= nlen;

        uint32_t const baseIndex = static_cast<uint32_t>(vertices.size() / kFloatsPerVertex);
        for (int k = 0; k < kFaceVerts; ++k) {
            uint32_t const vi = faces[f][k];
            vertices.push_back(rawVertices[vi][0] * scale);
            vertices.push_back(rawVertices[vi][1] * scale);
            vertices.push_back(rawVertices[vi][2] * scale);

            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);

            float const angle = PI_F * 0.5f + kTwoPi * k / kFaceVerts;
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
                              indices.data(), static_cast<uint32_t>(indices.size()));
}

std::shared_ptr<Mesh> Mesh::CreateIcosahedron(float size) {
    float const phi = (1.0f + std::sqrt(5.0f)) * 0.5f;
    // Scale so the circumradius (distance from center to any vertex) equals `size`.
    // Raw vertices have length sqrt(1 + phi^2) = sqrt(phi + 2).
    float const scale = size / std::sqrt(phi + 2.0f);

    float const rawVertices[12][3] = {
      { 0.0f,  1.0f,   phi},
      { 0.0f, -1.0f,   phi},
      { 0.0f,  1.0f,  -phi},
      { 0.0f, -1.0f,  -phi},
      { 1.0f,   phi,  0.0f},
      {-1.0f,   phi,  0.0f},
      { 1.0f,  -phi,  0.0f},
      {-1.0f,  -phi,  0.0f},
      {  phi,  0.0f,  1.0f},
      { -phi,  0.0f,  1.0f},
      {  phi,  0.0f, -1.0f},
      { -phi,  0.0f, -1.0f}
    };

    uint32_t const faces[20][3] = {
      {0,  8,  4},
      {0,  4,  5},
      {0,  5,  9},
      {0,  9,  1},
      {0,  1,  8},
      {3, 10,  2},
      {3,  2, 11},
      {3, 11,  7},
      {3,  7,  6},
      {3,  6, 10},
      {4,  8, 10},
      {4, 10,  2},
      {4,  2,  5},
      {5,  2, 11},
      {5, 11,  9},
      {9, 11,  7},
      {9,  7,  1},
      {1,  7,  6},
      {1,  6,  8},
      {8,  6, 10}
    };

    constexpr int kFaceCount       = 20;
    constexpr int kFaceVerts       = 3;
    constexpr int kFloatsPerVertex = 8;
    constexpr float kTwoPi         = 2.0f * PI_F;

    std::vector<float> vertices;
    vertices.reserve(kFaceCount * kFaceVerts * kFloatsPerVertex);
    std::vector<uint32_t> indices;
    indices.reserve(kFaceCount * kFaceVerts);

    for (int f = 0; f < kFaceCount; ++f) {
        // Flat-shaded: one outward normal per triangular face.
        uint32_t const i0 = faces[f][0], i1 = faces[f][1], i2 = faces[f][2];
        float const e1x = rawVertices[i1][0] - rawVertices[i0][0];
        float const e1y = rawVertices[i1][1] - rawVertices[i0][1];
        float const e1z = rawVertices[i1][2] - rawVertices[i0][2];
        float const e2x = rawVertices[i2][0] - rawVertices[i0][0];
        float const e2y = rawVertices[i2][1] - rawVertices[i0][1];
        float const e2z = rawVertices[i2][2] - rawVertices[i0][2];
        float nx        = e1y * e2z - e1z * e2y;
        float ny        = e1z * e2x - e1x * e2z;
        float nz        = e1x * e2y - e1y * e2x;
        // Flip if it points inward (icosahedron is convex + centered at origin).
        if (nx * rawVertices[i0][0] + ny * rawVertices[i0][1] + nz * rawVertices[i0][2] < 0.0f) {
            nx = -nx;
            ny = -ny;
            nz = -nz;
        }
        float const nlen = std::sqrt(nx * nx + ny * ny + nz * nz);
        nx /= nlen;
        ny /= nlen;
        nz /= nlen;

        uint32_t const baseIndex = static_cast<uint32_t>(vertices.size() / kFloatsPerVertex);
        for (int k = 0; k < kFaceVerts; ++k) {
            uint32_t const vi = faces[f][k];
            vertices.push_back(rawVertices[vi][0] * scale);
            vertices.push_back(rawVertices[vi][1] * scale);
            vertices.push_back(rawVertices[vi][2] * scale);

            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);

            float const angle = PI_F * 0.5f + kTwoPi * k / kFaceVerts;
            vertices.push_back(0.5f + 0.5f * std::cos(angle));
            vertices.push_back(0.5f + 0.5f * std::sin(angle));
        }

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
    }

    CORE_LOG_INFO("Icosahedron with {0} faces", indices.size() / 3);
    return CreateMeshFromData(vertices.data(),
                              static_cast<uint32_t>(vertices.size() * sizeof(float)),
                              indices.data(), static_cast<uint32_t>(indices.size()));
}

std::shared_ptr<Mesh> Mesh::CreateArrow(int segments) {
    // Unit arrow along +X: cylindrical shaft from x=0 to x=0.65, conical tip from x=0.65 to x=1.
    constexpr float shaftLen = 0.65f;
    constexpr float r        = 0.03f;  // shaft radius
    constexpr float R        = 0.14f;  // cone base radius
    float const TWO_PI       = 2.0f * std::numbers::pi_v<float>;

    std::vector<Vertex> verts;
    std::vector<uint32_t> idx;

    auto addVert = [&](Vec3 p, Vec3 n) { verts.push_back({p, n, {}}); };
    auto sz      = [&]() { return static_cast<uint32_t>(verts.size()); };

    // Disc cap in the YZ plane at xPos. faceRight → normal faces +X, otherwise −X.
    auto addCapX = [&](float xPos, float rad, bool faceRight) {
        Vec3 n     = {faceRight ? 1.0f : -1.0f, 0.0f, 0.0f};
        uint32_t c = sz();
        addVert({xPos, 0.0f, 0.0f}, n);
        uint32_t rim = sz();
        for (int i = 0; i < segments; ++i) {
            float t = TWO_PI * i / segments;
            addVert({xPos, rad * std::cos(t), rad * std::sin(t)}, n);
        }
        for (int i = 0; i < segments; ++i) {
            if (faceRight) idx.insert(idx.end(), {c, rim + i, rim + (i + 1) % segments});
            else idx.insert(idx.end(), {c, rim + (i + 1) % segments, rim + i});
        }
    };

    // Shaft cylinder
    for (int i = 0; i < segments; ++i) {
        float t = TWO_PI * i / segments, c = std::cos(t), s = std::sin(t);
        addVert({0.0f, r * c, r * s}, {0.0f, c, s});
    }
    for (int i = 0; i < segments; ++i) {
        float t = TWO_PI * i / segments, c = std::cos(t), s = std::sin(t);
        addVert({shaftLen, r * c, r * s}, {0.0f, c, s});
    }
    for (int i = 0; i < segments; ++i) {
        uint32_t b0 = i, b1 = (i + 1) % segments, t0 = segments + i,
                 t1 = segments + (i + 1) % segments;
        idx.insert(idx.end(), {b0, t0, t1, b0, t1, b1});
    }
    addCapX(0.0f, r, false);     // shaft back cap
    addCapX(shaftLen, r, true);  // shaft front cap (between shaft and cone)

    // Cone sides (flat-shaded)
    for (int i = 0; i < segments; ++i) {
        float t0 = TWO_PI * i / segments, t1 = TWO_PI * (i + 1) / segments;
        Vec3 v0    = {shaftLen, R * std::cos(t0), R * std::sin(t0)};
        Vec3 v1    = {shaftLen, R * std::cos(t1), R * std::sin(t1)};
        Vec3 v2    = {1.0f, 0.0f, 0.0f};
        Vec3 n     = (v1 - v0).crossProduct(v2 - v0).normalize();
        uint32_t b = sz();
        addVert(v0, n);
        addVert(v1, n);
        addVert(v2, n);
        idx.insert(idx.end(), {b, b + 1, b + 2});
    }
    addCapX(shaftLen, R, false);  // cone base cap

    return CreateMeshFromData(verts.data(), static_cast<uint32_t>(verts.size() * sizeof(Vertex)),
                              idx.data(), static_cast<uint32_t>(idx.size()));
}

std::shared_ptr<Mesh> Mesh::CreateCone(int segments) {
    // Unit cone: base circle (radius 1) in XY plane at z=0, tip at z=+1.
    float const TWO_PI = 2.0f * std::numbers::pi_v<float>;

    std::vector<Vertex> verts;
    std::vector<uint32_t> idx;

    auto addVert = [&](Vec3 p, Vec3 n) { verts.push_back({p, n, {}}); };

    // Lateral surface (flat-shaded)
    for (int i = 0; i < segments; ++i) {
        float t0 = TWO_PI * i / segments, t1 = TWO_PI * (i + 1) / segments;
        Vec3 v0    = {std::cos(t0), std::sin(t0), 0.0f};
        Vec3 v1    = {std::cos(t1), std::sin(t1), 0.0f};
        Vec3 v2    = {0.0f, 0.0f, 1.0f};
        Vec3 n     = (v1 - v0).crossProduct(v2 - v0).normalize();
        uint32_t b = static_cast<uint32_t>(verts.size());
        addVert(v0, n);
        addVert(v1, n);
        addVert(v2, n);
        idx.insert(idx.end(), {b, b + 1, b + 2});
    }

    // Base cap (in XY plane at z=0, normal −Z)
    uint32_t c = static_cast<uint32_t>(verts.size());
    addVert({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f});
    uint32_t rim = static_cast<uint32_t>(verts.size());
    for (int i = 0; i < segments; ++i) {
        float t = TWO_PI * i / segments;
        addVert({std::cos(t), std::sin(t), 0.0f}, {0.0f, 0.0f, -1.0f});
    }
    for (int i = 0; i < segments; ++i)
        idx.insert(idx.end(), {c, rim + (i + 1) % segments, rim + i});

    return CreateMeshFromData(verts.data(), static_cast<uint32_t>(verts.size() * sizeof(Vertex)),
                              idx.data(), static_cast<uint32_t>(idx.size()));
}
