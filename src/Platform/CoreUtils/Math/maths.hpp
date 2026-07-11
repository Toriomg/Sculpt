#pragma once
#include "matx.hpp"
#include "vec4.hpp"
#include "quaternion.hpp"

Vec3 rotateVec3(const Vec3& v, const Vec3& axis, float angle);

Matx4f QuatRotation(const Quaternion& q);