#include "Platform/CoreUtils/Math/maths.hpp"
#include <gtest/gtest.h>
#include <numbers>

namespace {

    constexpr float EPS = 1e-5f;

    void expectVec3Near(Vec3 actual, Vec3 expected) {
        EXPECT_NEAR(actual.x, expected.x, EPS);
        EXPECT_NEAR(actual.y, expected.y, EPS);
        EXPECT_NEAR(actual.z, expected.z, EPS);
    }

    void expectMatNear(Matx4f const& actual, Matx4f const& expected) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                EXPECT_NEAR(actual.m[i][j], expected.m[i][j], EPS)
                    << "at [" << i << "][" << j << "]";
            }
        }
    }

    // ---- Vec2 ----------------------------------------------------------------

    TEST(Vec2, Arithmetic) {
        Vec2 a{3.0f, 4.0f}, b{1.0f, 2.0f};
        EXPECT_NEAR((a + b).x, 4.0f, EPS);
        EXPECT_NEAR((a + b).y, 6.0f, EPS);
        EXPECT_NEAR((a - b).x, 2.0f, EPS);
        EXPECT_NEAR((a - b).y, 2.0f, EPS);
        EXPECT_NEAR((a * 2.0f).x, 6.0f, EPS);
        EXPECT_NEAR((a * 2.0f).y, 8.0f, EPS);
        EXPECT_NEAR((a / 2.0f).x, 1.5f, EPS);
        EXPECT_NEAR((a / 2.0f).y, 2.0f, EPS);
        EXPECT_NEAR((-a).x, -3.0f, EPS);
        EXPECT_NEAR((-a).y, -4.0f, EPS);
    }

    TEST(Vec2, CompoundAssignment) {
        Vec2 v{1.0f, 1.0f}, b{1.0f, 2.0f};
        v += b;
        EXPECT_NEAR(v.x, 2.0f, EPS);
        EXPECT_NEAR(v.y, 3.0f, EPS);
        v -= b;
        EXPECT_NEAR(v.x, 1.0f, EPS);
        EXPECT_NEAR(v.y, 1.0f, EPS);
        v *= 3.0f;
        EXPECT_NEAR(v.x, 3.0f, EPS);
        EXPECT_NEAR(v.y, 3.0f, EPS);
    }

    TEST(Vec2, DotProduct) {
        EXPECT_NEAR(Vec2(3, 4).dotProduct(Vec2(1, 2)), 11.0f, EPS);  // 3*1 + 4*2
    }

    TEST(Vec2, Length) {
        EXPECT_NEAR(Vec2(3, 4).length(), 5.0f, EPS);
    }

    TEST(Vec2, Normalize) {
        Vec2 n = Vec2(3.0f, 4.0f).normalize();
        EXPECT_NEAR(n.length(), 1.0f, EPS);
        EXPECT_NEAR(n.x, 3.0f / 5.0f, EPS);
        EXPECT_NEAR(n.y, 4.0f / 5.0f, EPS);
    }

    TEST(Vec2, NormalizeZeroVector) {
        Vec2 n = Vec2(0, 0).normalize();
        EXPECT_NEAR(n.x, 0.0f, EPS);
        EXPECT_NEAR(n.y, 0.0f, EPS);
    }

    // ---- Vec3 ----------------------------------------------------------------

    TEST(Vec3, Arithmetic) {
        Vec3 x{1, 0, 0}, y{0, 1, 0};
        expectVec3Near(x + y, {1, 1, 0});
        expectVec3Near(x - y, {1, -1, 0});
        expectVec3Near(x * 2.0f, {2, 0, 0});
        expectVec3Near(x / 2.0f, {0.5f, 0, 0});
        expectVec3Near(-x, {-1, 0, 0});
        expectVec3Near(x * y, {0, 0, 0});  // component-wise
    }

    TEST(Vec3, CompoundAssignment) {
        Vec3 v{1, 2, 3};
        v += {1, 1, 1};
        expectVec3Near(v, {2, 3, 4});
        v -= {1, 1, 1};
        expectVec3Near(v, {1, 2, 3});
    }

    TEST(Vec3, CrossProduct) {
        Vec3 x{1, 0, 0}, y{0, 1, 0}, z{0, 0, 1};
        expectVec3Near(x.crossProduct(y), z);
        expectVec3Near(y.crossProduct(z), x);
        expectVec3Near(z.crossProduct(x), y);
        expectVec3Near(y.crossProduct(x), {0, 0, -1});  // anti-commutativity
    }

    TEST(Vec3, DotProduct) {
        EXPECT_NEAR(Vec3(1, 0, 0).dotProduct(Vec3(0, 1, 0)), 0.0f, EPS);
        EXPECT_NEAR(Vec3(1, 0, 0).dotProduct(Vec3(1, 0, 0)), 1.0f, EPS);
        EXPECT_NEAR(Vec3(1, 2, 3).dotProduct(Vec3(4, 5, 6)), 32.0f, EPS);
    }

    TEST(Vec3, Length) {
        EXPECT_NEAR(Vec3(3, 4, 0).length(), 5.0f, EPS);
        EXPECT_NEAR(Vec3(1, 2, 2).length(), 3.0f, EPS);
    }

    TEST(Vec3, Normalize) {
        Vec3 n = Vec3(0, 3, 4).normalize();
        EXPECT_NEAR(n.length(), 1.0f, EPS);
    }

    TEST(Vec3, NormalizeZeroVector) {
        expectVec3Near(Vec3(0, 0, 0).normalize(), {0, 0, 0});
    }

    TEST(Vec3, PromotionFromVec2) {
        Vec3 v{
          Vec2{1.0f, 2.0f}
        };
        EXPECT_NEAR(v.z, 0.0f, EPS);
    }

    // ---- Vec4 ----------------------------------------------------------------

    TEST(Vec4, Arithmetic) {
        Vec4 a{1, 2, 3, 4};
        EXPECT_NEAR((a + Vec4{1, 1, 1, 1}).x, 2.0f, EPS);
        EXPECT_NEAR((a - Vec4{1, 1, 1, 1}).w, 3.0f, EPS);
        EXPECT_NEAR((a * 2.0f).z, 6.0f, EPS);
        EXPECT_NEAR((a / 2.0f).y, 1.0f, EPS);
    }

    TEST(Vec4, Length) {
        EXPECT_NEAR(Vec4(1, 2, 3, 4).length(), std::sqrt(30.0f), EPS);
    }

    TEST(Vec4, Normalize) {
        EXPECT_NEAR(Vec4(1, 2, 3, 4).normalize().length(), 1.0f, EPS);
    }

    TEST(Vec4, NormalizeZeroVector) {
        EXPECT_NEAR(Vec4(0, 0, 0, 0).normalize().length(), 0.0f, EPS);
    }

    TEST(Vec4, PromotionFromVec3) {
        Vec4 v{
          Vec3{1, 2, 3}
        };
        EXPECT_NEAR(v.w, 0.0f, EPS);
    }

    // ---- Matx4f --------------------------------------------------------------

    TEST(Matx4f, Identity) {
        Matx4f id = Matx4f::identity();
        EXPECT_EQ(id * id, id);
        expectVec3Near(id.transformPoint({3, 5, 7}), {3, 5, 7});
    }

    TEST(Matx4f, Translation) {
        Matx4f t = Matx4f::translation({1, 2, 3});
        expectVec3Near(t.transformPoint({0, 0, 0}), {1, 2, 3});
        expectVec3Near(t.transformPoint({1, 1, 1}), {2, 3, 4});
    }

    TEST(Matx4f, Scaling) {
        expectVec3Near(Matx4f::scaling({2, 3, 4}).transformPoint({1, 1, 1}), {2, 3, 4});
        expectVec3Near(Matx4f::scalingScalar(2.0f).transformPoint({1, 1, 1}), {2, 2, 2});
    }

    TEST(Matx4f, RotationX) {
        // degrees; Y-axis maps to Z-axis
        expectVec3Near(Matx4f::rotationX(90.0f).transformPoint({0, 1, 0}), {0, 0, 1});
        expectVec3Near(Matx4f::rotationX(360.0f).transformPoint({1, 2, 3}), {1, 2, 3});
    }

    TEST(Matx4f, RotationY) {
        // degrees; X-axis maps to -Z-axis
        expectVec3Near(Matx4f::rotationY(90.0f).transformPoint({1, 0, 0}), {0, 0, -1});
    }

    TEST(Matx4f, RotationZ) {
        // degrees; X-axis maps to Y-axis
        expectVec3Near(Matx4f::rotationZ(90.0f).transformPoint({1, 0, 0}), {0, 1, 0});
    }

    TEST(Matx4f, RotationAxisAngle) {
        // takes radians, unlike rotationX/Y/Z which take degrees
        float pi2 = std::numbers::pi_v<float> * 0.5f;
        expectVec3Near(Matx4f::rotation(Vec3{0, 0, 1}.normalize(), pi2).transformPoint({1, 0, 0}),
                       {0, 1, 0});
    }

    TEST(Matx4f, RotationEuler) {
        // rotation(x,y,z) == rotZ * rotY * rotX
        Matx4f combined = Matx4f::rotation(30.0f, 45.0f, 60.0f);
        Matx4f expected =
            Matx4f::rotationZ(60.0f) * Matx4f::rotationY(45.0f) * Matx4f::rotationX(30.0f);
        expectMatNear(combined, expected);
    }

    TEST(Matx4f, MultiplyNonCommutative) {
        Matx4f t = Matx4f::translation({1, 0, 0});
        Matx4f s = Matx4f::scaling({2, 2, 2});
        // t*s: scale origin to origin, then translate → (1,0,0)
        expectVec3Near((t * s).transformPoint({0, 0, 0}), {1, 0, 0});
        // s*t: translate origin to (1,0,0), then scale → (2,0,0)
        expectVec3Near((s * t).transformPoint({0, 0, 0}), {2, 0, 0});
    }

    TEST(Matx4f, MultiplyVec4) {
        Vec4 r = Matx4f::identity() * Vec4{1, 2, 3, 4};
        EXPECT_NEAR(r.x, 1.0f, EPS);
        EXPECT_NEAR(r.y, 2.0f, EPS);
        EXPECT_NEAR(r.z, 3.0f, EPS);
        EXPECT_NEAR(r.w, 4.0f, EPS);
    }

    TEST(Matx4f, Orthographic) {
        Matx4f o = Matx4f::orthographic(-1, 1, -1, 1, 0.1f, 100.0f);
        EXPECT_NEAR((o * Vec4{1, 0, 0, 1}).x, 1.0f, EPS);
        EXPECT_NEAR((o * Vec4{0, 1, 0, 1}).y, 1.0f, EPS);
    }

    TEST(Matx4f, PerspectiveWComponent) {
        // w == -z for any point in front of the camera (right-handed convention)
        Matx4f p = Matx4f::perspective(90.0f, 1.0f, 0.1f, 100.0f);
        Vec4 pt  = p * Vec4{0, 0, -5, 1};
        EXPECT_NEAR(pt.w, 5.0f, EPS);
    }

    // ---- Quaternion ----------------------------------------------------------

    TEST(Quaternion, IdentityToMatrix) {
        expectMatNear(QuatRotation(Quaternion::identity()), Matx4f::identity());
    }

    TEST(Quaternion, Conjugate) {
        Quaternion q{0.5f, 1.0f, 2.0f, 3.0f};
        Quaternion c = q.conjugate();
        EXPECT_NEAR(c.w, 0.5f, EPS);
        EXPECT_NEAR(c.x, -1.0f, EPS);
        EXPECT_NEAR(c.y, -2.0f, EPS);
        EXPECT_NEAR(c.z, -3.0f, EPS);
    }

    TEST(Quaternion, MultiplyByConjugateIsScalar) {
        Quaternion q{0.5f, 1.0f, 2.0f, 3.0f};
        Quaternion qq = q * q.conjugate();
        EXPECT_NEAR(qq.x, 0.0f, EPS);
        EXPECT_NEAR(qq.y, 0.0f, EPS);
        EXPECT_NEAR(qq.z, 0.0f, EPS);
    }

    TEST(Quaternion, Normalize) {
        EXPECT_NEAR(Quaternion(0.5f, 1.0f, 2.0f, 3.0f).normalize().length(), 1.0f, EPS);
    }

    TEST(Quaternion, NormalizeZeroReturnsIdentity) {
        EXPECT_NEAR(Quaternion(0, 0, 0, 0).normalize().w, 1.0f, EPS);
    }

    TEST(Quaternion, RotationMatchesMatrixRotation) {
        // 90° around Z via quat should match rotationZ(90°)
        float pi2 = std::numbers::pi_v<float> * 0.5f;
        Quaternion qz({0, 0, 1}, pi2);
        expectMatNear(QuatRotation(qz.normalize()), Matx4f::rotationZ(90.0f));
    }

    // ---- rotateVec3 ----------------------------------------------------------

    TEST(RotateVec3, BasisVectors) {
        float pi2 = std::numbers::pi_v<float> * 0.5f;
        expectVec3Near(rotateVec3({1, 0, 0}, {0, 0, 1}, pi2), {0, 1, 0});  // X→Y around Z
        expectVec3Near(rotateVec3({0, 1, 0}, {1, 0, 0}, pi2), {0, 0, 1});  // Y→Z around X
        expectVec3Near(rotateVec3({0, 0, 1}, {0, 1, 0}, pi2), {1, 0, 0});  // Z→X around Y
    }

    TEST(RotateVec3, ZeroAngle) {
        expectVec3Near(rotateVec3({1, 2, 3}, {0, 0, 1}, 0.0f), {1, 2, 3});
    }

    TEST(RotateVec3, ConsistentWithQuatRotationMatrix) {
        float pi2   = std::numbers::pi_v<float> * 0.5f;
        Vec3 v      = {1, 1, 0};
        Vec3 byFunc = rotateVec3(v, {0, 0, 1}, pi2);
        Matx4f mz   = QuatRotation(Quaternion({0, 0, 1}, pi2).normalize());
        expectVec3Near(byFunc, mz.transformPoint(v));
    }

}  // namespace
