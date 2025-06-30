#pragma once
#include "vec3.h"

class Quat {
public:
	float w, x, y, z;

	Quat() {
		w = 1.0f;
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	Quat(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

	Quat(const Vec3& axis, float angle) {
		float halfAngle = angle * 0.5f;
		float s = sin(halfAngle);
		w = cos(halfAngle);
		x = axis.x * s;
		y = axis.y * s;
		z = axis.z * s;
	}

	Vec3 getVector() const {
		return Vec3(x, y, z);
	}

	Quat conjugate(const Quat& q) const {
		return Quat(q.w, -q.x, -q.y, -q.z);
	}

	Quat operator*(const Quat& q) const {
		return Quat(
			w * q.w - x * q.x - y * q.y - z * q.z,
			w * q.x + x * q.w + y * q.z - z * q.y,
			w * q.y - x * q.z + y * q.w + z * q.x,
			w * q.z + x * q.y - y * q.x + z * q.w
		);
	}

	float length() const {
		return sqrt(w * w + x * x + y * y + z * z);
	}

};