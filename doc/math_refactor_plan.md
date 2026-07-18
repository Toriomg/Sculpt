# Math Library Refactor Plan — Generic Vec<N> / Mat<N,M>

## Goal

Replace the four hand-rolled classes (`Vec2`, `Vec3`, `Vec4`, `Matx4f`) with two generic
templates (`Vec<N>` and `Mat<N,M>`), eliminating the duplicated arithmetic while keeping the
public API identical so no call sites change.

---

## Constraints discovered before designing

- **132 uses of `.x` / `.y` / `.z` / `.w`** across the codebase as direct field access.
- **`&pos.x` is passed as `float*` to ImGui** (`ImGui::DragFloat3(..., &pos.x, ...)`).
  This means `x`, `y`, `z`, `w` must be real `float` fields in memory — accessor methods
  (`x()`) are ruled out.
- **261 uses of the type names** `Vec2`, `Vec3`, `Vec4`, `Matx4f` — handled cheaply with
  `using` aliases, zero call-site changes.
- Method names (`dotProduct`, `crossProduct`, `normalize`, `length`) are used outside the
  math directory and must be preserved.

---

## Central design decision: union storage

The only approach that satisfies all constraints without touching 132 call sites:

```cpp
// inside Vec<3>:
union {
    float data[3];       // used by generic loop-based operations
    struct { float x, y, z; };   // anonymous struct — C extension
};
```

`data[i]` provides array indexing for generic arithmetic.
`x / y / z` provide named field access and real addresses for ImGui.

**The anonymous struct inside a union is a C extension, not standard C++.**
It is fully supported by Clang (the project compiler) and is the canonical solution
used by GLM, DirectXMath, and virtually every production 3D math library.
A single targeted `#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"` is placed
around the header, with a comment explaining why.

Alternatives considered and rejected:

| Option | Problem |
|---|---|
| `x()` / `y()` accessor methods | `&pos.x` breaks; 132 call sites must change |
| `float& x = data[0]` reference member | Adds pointer-sized overhead per field; makes class non-trivially copyable |
| `(&x)[i]` pointer arithmetic across fields | UB per C++ standard (even if compilers accept it) |
| Full separate specializations without CRTP | Duplicates all arithmetic — the original problem |

---

## New file structure

```
src/Platform/CoreUtils/Math/
├── vec.hpp        ← replaces vec2.hpp + vec3.hpp + vec4.hpp
├── mat.hpp        ← replaces matx.hpp
├── quaternion.hpp ← update include path only
├── maths.hpp      ← update includes
└── maths.cpp      ← untouched
```

`vec2.hpp`, `vec3.hpp`, `vec4.hpp`, `matx.hpp` are deleted.

---

## `vec.hpp` — design

### CRTP base `VecOps<Derived, N>`

All generic operations live here exactly once. They access `Derived::data[N]` (the union
array member) via a cast to the derived type.

```cpp
template<typename D, std::size_t N>
struct VecOps {
    D     operator+(D const&)  const;
    D     operator-(D const&)  const;
    D     operator*(float)     const;
    D     operator/(float)     const;
    D     operator-()          const;
    bool  operator==(D const&) const;
    float length()             const;
    D     normalize()          const;
    float dot(D const&)        const;
};
```

### Specializations

Only `Vec<2>`, `Vec<3>`, `Vec<4>` are valid instantiations — the primary template is
declared but not defined, so any other `Vec<5>` etc. fails to compile.

```
Vec<2>  union { float data[2]; struct { float x, y; }; }
        + component-wise operator*(Vec<2>)
        + dotProduct(Vec<2>) — alias for dot(), preserves existing API
        + compound assignment operators +=, -=, *=

Vec<3>  union { float data[3]; struct { float x, y, z; }; }
        + Vec2 promotion constructor
        + crossProduct(Vec<3>)
        + dotProduct(Vec<3>) alias
        + component-wise operator*(Vec<3>)
        + compound assignment operators +=, -=

Vec<4>  union { float data[4]; struct { float x, y, z, w; }; }
        + Vec2 and Vec3 promotion constructors
        + dotProduct(Vec<4>) alias (note: currently returns Vec<4> component-wise —
          this is a pre-existing naming bug, out of scope for this refactor)
        + compound assignment operators +=, -=
```

### Aliases

```cpp
using Vec2 = Vec<2>;
using Vec3 = Vec<3>;
using Vec4 = Vec<4>;
```

Zero changes to the 261 type-name call sites.

---

## `mat.hpp` — design

### Generic `Mat<N, M = N>`

A single template covers all sizes. The 4×4-only factory methods are gated with `requires`
instead of a separate specialization — this gives a clear compile error if called on the
wrong size and avoids duplicating the generic operators.

```cpp
template<std::size_t N, std::size_t M = N>
struct Mat {
    float data[N][M]{};

    // Always available — dimensions enforced by template parameter matching
    template<std::size_t P>
    Mat<N, P> operator*(Mat<M, P> const&) const;   // size-safe: N×M * M×P → N×P
    Mat       operator*(float scalar)      const;
    Vec<N>    operator*(Vec<M> const&)    const;
    bool      operator==(Mat const&)      const;
    bool      operator!=(Mat const&)      const;
    float const* data_ptr() const;                 // for glUniformMatrix4fv

    // Square matrices only
    static Mat identity() requires(N == M);

    // 4×4 only — compile error if called on any other size
    Vec<3> transformPoint(Vec<3> const&)                    const requires(N == 4 && M == 4);
    static Mat translation(Vec<3> const&)                        requires(N == 4 && M == 4);
    static Mat scaling(Vec<3> const&)                            requires(N == 4 && M == 4);
    static Mat scalingScalar(float)                              requires(N == 4 && M == 4);
    static Mat rotationX(float degrees)                          requires(N == 4 && M == 4);
    static Mat rotationY(float degrees)                          requires(N == 4 && M == 4);
    static Mat rotationZ(float degrees)                          requires(N == 4 && M == 4);
    static Mat rotation(Vec<3> axis, float radians)              requires(N == 4 && M == 4);
    static Mat rotation(float xDeg, float yDeg, float zDeg)     requires(N == 4 && M == 4);
    static Mat lookAt(Vec<3> pos, Vec<3> target, Vec<3> up)      requires(N == 4 && M == 4);
    static Mat orthographic(float l, float r, float b, float t,
                            float zNear, float zFar)             requires(N == 4 && M == 4);
    static Mat perspective(float fov, float aspect,
                           float zNear, float zFar)              requires(N == 4 && M == 4);
};

using Matx4f = Mat<4>;
```

---

## Known inconsistency — document, do not fix

`rotationX/Y/Z(angle)` take **degrees** and convert internally via `radians()`.
`rotation(axis, angle)` and `Quaternion(axis, angle)` take **radians** directly.

This pre-exists and fixing it would silently break all call sites. Both overloads will
carry a comment making the unit explicit.

---

## Execution order

1. Write `vec.hpp` — CRTP base + 3 specializations + `using` aliases
2. Write `mat.hpp` — `Mat<N,M>` template + `Matx4f` alias
3. Update `quaternion.hpp` — change `#include "vec3.hpp"` to `#include "vec.hpp"`
4. Update `maths.hpp` — change includes
5. Delete `vec2.hpp`, `vec3.hpp`, `vec4.hpp`, `matx.hpp`
6. Build — fix any errors
7. Run `./build/MathTest` — must stay green

---

## What does not change

| Item | Reason |
|---|---|
| All `.x` / `.y` / `.z` / `.w` field access | union members, same layout |
| `&pos.x` passed to ImGui | same real `float` field address |
| `dotProduct`, `crossProduct` method names | kept on specializations |
| `maths.cpp` | `rotateVec3` and `QuatRotation` are untouched |
| `Quaternion` internals | only its `#include` changes |
| All 261 type-name usages | covered by `using` aliases |
| Tests in `tests/math_test.cpp` | serve as regression guard throughout |
