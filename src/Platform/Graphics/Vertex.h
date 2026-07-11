#pragma once
#include "Platform/CoreUtils/Math/maths.h"

class Vertex {
public:
	Vec3 pos;
	Vec3 normal;
	Vec2 texCoord;

	Vertex() : pos(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 1.0f), texCoord(0.0f, 0.0f) {}
	Vertex(const Vec3& position, const Vec3& normalVector, const Vec2& textureCoord)
		: pos(position), normal(normalVector), texCoord(textureCoord) {
	}
};