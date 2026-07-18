// Math utility aggregate: re-exports Matx4f, Vec types, and Quaternion; provides radians() and
// QuatRotation().
#pragma once
#include "Platform/CoreUtils/Math/mat.hpp"
#include "Platform/CoreUtils/Math/quaternion.hpp"

Vec3 rotateVec3(Vec3 const& v, Vec3 const& axis, float angle);

Matx4f QuatRotation(Quaternion const& q);
