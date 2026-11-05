#pragma once

#include <algorithm>
#include <cmath>
#include "Vector3.h"
#include "Consts.h"

struct Matrix4
{
    float m[4][4]{};

    // 単位行列
    static constexpr Matrix4 CreateIdentity() noexcept
    {
        Matrix4 result{};
        result.m[0][0] = 1.0f;
        result.m[1][1] = 1.0f;
        result.m[2][2] = 1.0f;
        result.m[3][3] = 1.0f;
        return result;
    }

    // 移動行列
    static constexpr Matrix4 CreateTranslation(Vec3 pos) noexcept
    {
        Matrix4 result = CreateIdentity();
        result.m[3][0] = pos.x;
        result.m[3][1] = pos.y;
        result.m[3][2] = pos.z;
        return result;
    }

    // スケール行列
    static constexpr Matrix4 CreateScale(Vec3 scale) noexcept
    {
        Matrix4 result = CreateIdentity();
        result.m[0][0] = scale.x;
        result.m[1][1] = scale.y;
        result.m[2][2] = scale.z;
        return result;
    }

    // X軸回転
    static Matrix4 CreateRotationX(float angle) noexcept
    {
        Matrix4 result = CreateIdentity();
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        result.m[1][1] = c;
        result.m[1][2] = s;
        result.m[2][1] = -s;
        result.m[2][2] = c;
        return result;
    }

    // Y軸回転
    static Matrix4 CreateRotationY(float angle) noexcept
    {
        Matrix4 result = CreateIdentity();
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        result.m[0][0] = c;
        result.m[0][2] = -s;
        result.m[2][0] = s;
        result.m[2][2] = c;
        return result;
    }

    // Z軸回転
    static Matrix4 CreateRotationZ(float angle) noexcept
    {
        Matrix4 result = CreateIdentity();
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        result.m[0][0] = c;
        result.m[0][1] = s;
        result.m[1][0] = -s;
        result.m[1][1] = c;
        return result;
    }

    // XYZの順に回転させる
    static Matrix4 CreateRotationXYZ(Vec3 rotation) noexcept
    {
        return CreateRotationX(rotation.x)
            * CreateRotationY(rotation.y)
            * CreateRotationZ(rotation.z);
    }

    // 行列積
    Matrix4 operator*(const Matrix4& other) const noexcept
    {
        Matrix4 result{};

        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                for (int k = 0; k < 4; ++k)
                {
                    result.m[row][col] +=
                        m[row][k] * other.m[k][col];
                }
            }
        }

        return result;
    }

    // 正しい行列積で自身を更新
    Matrix4& operator*=(const Matrix4& other) noexcept
    {
        *this = *this * other;
        return *this;
    }

    // 位置の取得
    Vec3 GetTranslation() const noexcept
    {
        return {
            m[3][0],
            m[3][1],
            m[3][2]
        };
    }

    // 位置の設定
    void SetTranslation(Vec3 position) noexcept
    {
        m[3][0] = position.x;
        m[3][1] = position.y;
        m[3][2] = position.z;
    }

    // スケールの取得
    Vec3 GetScale() const noexcept
    {
        return {
            std::sqrt(m[0][0] * m[0][0]
                    + m[0][1] * m[0][1]
                    + m[0][2] * m[0][2]),

            std::sqrt(m[1][0] * m[1][0]
                    + m[1][1] * m[1][1]
                    + m[1][2] * m[1][2]),

            std::sqrt(m[2][0] * m[2][0]
                    + m[2][1] * m[2][1]
                    + m[2][2] * m[2][2])
        };
    }

    // 回転の取得
    // 正のスケールを前提にする
    Vec3 GetRotationXYZ() const noexcept
    {
        const Vec3 scale = GetScale();

        if (scale.x < Const::EPS ||
            scale.y < Const::EPS ||
            scale.z < Const::EPS)
        {
            return {};
        }

        // スケールの影響を除去
        const float r00 = m[0][0] / scale.x;
        const float r01 = m[0][1] / scale.x;
        const float r02 = m[0][2] / scale.x;

        const float r10 = m[1][0] / scale.y;
        const float r11 = m[1][1] / scale.y;
        const float r12 = m[1][2] / scale.y;

        const float r22 = m[2][2] / scale.z;

        const float y =
            std::asin(std::clamp(-r02, -1.0f, 1.0f));

        if (std::abs(std::cos(y)) > Const::EPS)
        {
            return {
                std::atan2(r12, r22),
                y,
                std::atan2(r01, r00)
            };
        }

        // ジンバルロック付近
        const float x = (y >= 0.0f)
            ? std::atan2(r10, r11)
            : std::atan2(-r10, r11);

        return { x, y, 0.0f };
    }

    // 回転・スケールを適用する。平行移動は適用しない
    Vec3 TransformVector(Vec3 value) const noexcept
    {
        return {
            value.x * m[0][0]
                + value.y * m[1][0]
                + value.z * m[2][0],

            value.x * m[0][1]
                + value.y * m[1][1]
                + value.z * m[2][1],

            value.x * m[0][2]
                + value.y * m[1][2]
                + value.z * m[2][2]
        };
    }

    // 回転・スケール・平行移動を適用する
    Vec3 TransformPoint(Vec3 value) const noexcept
    {
        Vec3 result = TransformVector(value);
        const Vec3 position = GetTranslation();

        result.x += position.x;
        result.y += position.y;
        result.z += position.z;

        return result;
    }
};
