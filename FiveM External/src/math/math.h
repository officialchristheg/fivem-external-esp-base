#pragma once
#include <numbers>
#include <cmath>
#include <Windows.h>
#include "../../ext/SimpleMath.h"

using namespace DirectX::SimpleMath;

class Vec2
{
public:
    constexpr Vec2(
        const float x = 0.f,
        const float y = 0.f) noexcept :
        x(x), y(y) {
    }
    float x, y;
};

class Vec3
{
public:
    constexpr Vec3(
        const float x = 0.f,
        const float y = 0.f,
        const float z = 0.f) noexcept :
        x(x), y(y), z(z) {
    }

    constexpr Vec3 operator-(const Vec3& other) const noexcept {
        return Vec3{ x - other.x, y - other.y, z - other.z };
    }
    constexpr Vec3 operator+(const Vec3& other) const noexcept {
        return Vec3{ x + other.x, y + other.y, z + other.z };
    }
    constexpr const Vec3& operator*(const float factor) const noexcept;
    constexpr const Vec3& operator/(const float factor) const noexcept;

    const Vec3 Transform(Matrix* mIn);

    float Distance(Vec3 a, Vec3 b);
    Vec3 CalculateAngle(Vec3 src, Vec3 dst);
    const bool IsZero();

    const bool world_to_screen(const DirectX::SimpleMath::Matrix& view_matrix, Vec2& out);

    float x, y, z;
};
