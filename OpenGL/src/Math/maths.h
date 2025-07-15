#pragma once
#include "matx.h"
#include "vec4.h"
#include "quaternion.h"

Vec3 rotateVec3(const Vec3& v, const Vec3& axis, float angle);

Matx4f QuatRotation(const Quaternion& q);