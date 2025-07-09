#include "maths.h"

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