# Math API Reference
> `#include "Platform/CoreUtils/Math/maths.hpp"` — never mix with GLM.

## Types
| Type | Alias | Storage |
|---|---|---|
| `Vec2` | `Vec<2>` | `float x, y` + `float data[2]` |
| `Vec3` | `Vec<3>` | `float x, y, z` + `float data[3]` |
| `Vec4` | `Vec<4>` | `float x, y, z, w` + `float data[4]` |
| `Matx4f` | `Mat<4,4>` | `float m[4][4]` row-major; pass to GL via `data_ptr()` |
| `Quaternion` | — | `float w, x, y, z` |

## Vec — common ops (all types)
| Op / Method | Returns | Notes |
|---|---|---|
| `a + b`, `a - b` | `VecN` | component-wise |
| `a * scalar`, `a / scalar` | `VecN` | scalar ops |
| `a * b` | `VecN` | component-wise multiply |
| `-a` | `VecN` | negate |
| `a == b` | `bool` | exact float compare |
| `a.length()` | `float` | Euclidean length |
| `a.normalize()` | `VecN` | returns zero-vec if len < 1e-6 |
| `a.dot(b)` | `float` | dot product |
| `+=`, `-=`, `*=` | — | in-place (Vec2, Vec3, Vec4) |

## Vec3 extras
| Method | Returns |
|---|---|
| `a.crossProduct(b)` | `Vec3` — right-hand cross product |
| `a.dotProduct(b)` | `float` — alias for `dot()` |

> **Vec4 quirk:** `Vec4::dotProduct(b)` returns `Vec4` (component-wise), not `float`. Use `.dot(b)` for a scalar.

## Matx4f factories
All are `static`, `requires Mat4<4,4>`.

| Factory | Description |
|---|---|
| `Matx4f::identity()` | 4×4 identity |
| `Matx4f::translation(Vec3 v)` | Translation |
| `Matx4f::scaling(Vec3 v)` | Non-uniform scale |
| `Matx4f::scalingScalar(float s)` | Uniform scale |
| `Matx4f::rotationX(float deg)` | Rotate X — takes **degrees** |
| `Matx4f::rotationY(float deg)` | Rotate Y — takes **degrees** |
| `Matx4f::rotationZ(float deg)` | Rotate Z — takes **degrees** |
| `Matx4f::rotation(float xDeg, float yDeg, float zDeg)` | Euler ZYX — takes **degrees** |
| `Matx4f::rotation(Vec3 axis, float angle)` | Axis-angle — takes **radians** |
| `Matx4f::lookAt(pos, target, up)` | View matrix |
| `Matx4f::perspective(fov, aspect, near, far)` | Perspective — `fov` in degrees |
| `Matx4f::orthographic(l, r, b, t, near, far)` | Orthographic projection |

## Matx4f operations
| Op | Description |
|---|---|
| `A * B` | Matrix multiply; inner dims checked at compile time |
| `A * Vec4` | Transform column vector; Vec dim must match M |
| `A.transformPoint(Vec3 p)` | Homogeneous multiply + w-divide (for 3D points) |
| `A.data_ptr()` | `const float*` to `m[0][0]` — use with `glUniformMatrix4fv` |
| `A == B`, `A != B` | Exact float compare |

## Quaternion
```cpp
Quaternion(Vec3 axis, float angle_radians); // axis-angle ctor
Quaternion(float w, float x, float y, float z);
Quaternion::identity()         // (1,0,0,0)
q * q2                         // Hamilton product
q.conjugate()                  // inverse for unit quaternions
q.normalize()
q.length()
q.getVector()                  // returns Vec3(x,y,z)
QuatRotation(q)                // → Matx4f  (free function in maths.hpp)
rotateVec3(v, axis, angle_rad) // → Vec3    (free function in maths.hpp)
```

## Utility functions
```cpp
float radians(float degrees);      // inline, defined in mat.hpp
inline constexpr float PI_F;       // = std::numbers::pi_v<float>
```

## TransformComponent matrix order
```cpp
GetMatrix() = translation(Translation)
            * rotation(EulerDegrees.x, EulerDegrees.y, EulerDegrees.z)  // ZYX, degrees
            * scaling(Scale)
```
Rotation is applied Z → Y → X (standard Euler, applied right-to-left on column vectors).
