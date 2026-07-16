// Interleaved vertex struct (position, normal, UV) matching the standard shader input layout; used
// by Mesh factories.
#pragma once
#include "Platform/CoreUtils/Math/maths.hpp"

class Vertex {
public:
    Vec3 pos;
    Vec3 normal;
    Vec2 texCoord;

    Vertex() : pos(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 1.0f), texCoord(0.0f, 0.0f) { }
    Vertex(Vec3 const& position, Vec3 const& normalVector, Vec2 const& textureCoord)
        : pos(position), normal(normalVector), texCoord(textureCoord) { }
};
