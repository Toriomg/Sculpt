// Math utility aggregate: re-exports Matx4f, Vec types, and Quaternion; provides radians() and QuatRotation().
#pragma once
#include "matx.hpp"
#include "vec4.hpp"
#include "quaternion.hpp"

Vec3 rotateVec3(const Vec3& v, const Vec3& axis, float angle);

Matx4f    QuatRotation(const Quaternion& q);
Quaternion QuatFromEulerDegrees(const Vec3& eulerDegrees);
Vec3       QuatToEulerDegrees(const Quaternion& q);