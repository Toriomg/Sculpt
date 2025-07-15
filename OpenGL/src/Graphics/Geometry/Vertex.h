#pragma once
#include "../Math/maths.h"

class Vertex{
public:
	Vec3 pos; // x, y, z
	Vec3 texCoord; // u, v, w
	Vec3 normal;   // nx, ny, nz

	Vertex() : pos(0.0f, 0.0f, 0.0f), texCoord(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 1.0f) {}
	Vertex(const Vec3& position, const Vec3& textureCoord, const Vec3& normalVector)
		: pos(position), texCoord(textureCoord), normal(normalVector) {
	}

	Vertex(float x, float y, float z, float u, float v, float w, float nx, float ny, float nz)
		: pos(x, y, z), texCoord(u, v, w), normal(nx, ny, nz) {
	}
};