#include "maths.hpp"
#include <cmath>

Vec3 rotateVec3(const Vec3& v, const Vec3& axis, float angle) {
    Quaternion rotationQ(axis, angle);
    Quaternion conjugateQ = rotationQ.conjugate();

    // Represent the vector 'v' to be rotated as a pure quaternion
    Quaternion p(0.0f, v.x, v.y, v.z);

    // Apply the rotation formula: p' = q * p * q_conjugate
    Quaternion p_rotated = rotationQ * p * conjugateQ;

    // The result is the vector part of the new quaternion
    return Vec3(p_rotated.x, p_rotated.y, p_rotated.z);
}

Matx4f QuatRotation(const Quaternion& q) {
	Matx4f result;

	float w = q.w;
	float x = q.x;
	float y = q.y;
	float z = q.z;

    // Pre-compute products to avoid 16 redundant multiplications in the matrix fill below.
    // The resulting matrix is the standard unit-quaternion-to-rotation-matrix formula.
	float xx = x * x;
	float yy = y * y;
	float zz = z * z;
	float xy = x * y;
	float xz = x * z;
	float yz = y * z;
	float wx = w * x;
	float wy = w * y;
	float wz = w * z;

	result.m[0][0] = 1.0f - 2.0f * (yy + zz);
	result.m[0][1] = 2.0f * (xy - wz);
	result.m[0][2] = 2.0f * (xz + wy);
	result.m[0][3] = 0.0f;

	result.m[1][0] = 2.0f * (xy + wz);
	result.m[1][1] = 1.0f - 2.0f * (xx + zz);
	result.m[1][2] = 2.0f * (yz - wx);
	result.m[1][3] = 0.0f;

	result.m[2][0] = 2.0f * (xz - wy);
	result.m[2][1] = 2.0f * (yz + wx);
	result.m[2][2] = 1.0f - 2.0f * (xx + yy);
	result.m[2][3] = 0.0f;

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}