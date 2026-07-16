#include "maths.hpp"
#include <cmath>

Vec3 rotateVec3(Vec3 const& v, Vec3 const& axis, float angle) {
    Quaternion const rotationQ(axis, angle);
    Quaternion const conjugateQ = rotationQ.conjugate();

    // Represent the vector 'v' to be rotated as a pure quaternion
    Quaternion const p(0.0f, v.x, v.y, v.z);

    // Apply the rotation formula: p' = q * p * q_conjugate
    Quaternion const p_rotated = rotationQ * p * conjugateQ;

    // The result is the vector part of the new quaternion
    return {p_rotated.x, p_rotated.y, p_rotated.z};
}

Matx4f QuatRotation(Quaternion const& q) {
    Matx4f result;

    float const w = q.w;
    float const x = q.x;
    float const y = q.y;
    float const z = q.z;

    // Pre-compute products to avoid 16 redundant multiplications in the matrix fill below.
    // The resulting matrix is the standard unit-quaternion-to-rotation-matrix formula.
    float const xx = x * x;
    float const yy = y * y;
    float const zz = z * z;
    float const xy = x * y;
    float const xz = x * z;
    float const yz = y * z;
    float const wx = w * x;
    float const wy = w * y;
    float const wz = w * z;

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
