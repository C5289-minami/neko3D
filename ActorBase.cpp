#include "ActorBase.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>


namespace
{
    using RotationMatrix = std::array<std::array<float, 3>, 3>;

    // ヘルパー関数: 回転を考慮してベクトルを回転させる
    Vec3 RotateXYZ(const Vec3& value, const Vec3& rotation)
    {
        const float cosX = std::cos(rotation.x);
        const float sinX = std::sin(rotation.x);
        const float cosY = std::cos(rotation.y);
        const float sinY = std::sin(rotation.y);
        const float cosZ = std::cos(rotation.z);
        const float sinZ = std::sin(rotation.z);

        const Vec3 rotatedX{
            value.x,
            value.y * cosX - value.z * sinX,
            value.y * sinX + value.z * cosX
        };
        const Vec3 rotatedY{
            rotatedX.x * cosY + rotatedX.z * sinY,
            rotatedX.y,
            -rotatedX.x * sinY + rotatedX.z * cosY
        };

        return Vec3{
            rotatedY.x * cosZ - rotatedY.y * sinZ,
            rotatedY.x * sinZ + rotatedY.y * cosZ,
            rotatedY.z
        };
    }

    RotationMatrix MakeRotationMatrix(const Vec3& rotation)
    {
        const float cosX = std::cos(rotation.x);
        const float sinX = std::sin(rotation.x);
        const float cosY = std::cos(rotation.y);
        const float sinY = std::sin(rotation.y);
        const float cosZ = std::cos(rotation.z);
        const float sinZ = std::sin(rotation.z);

        return { {
            { cosY * cosZ, sinX * sinY * cosZ - cosX * sinZ, cosX * sinY * cosZ + sinX * sinZ },
            { cosY * sinZ, sinX * sinY * sinZ + cosX * cosZ, cosX * sinY * sinZ - sinX * cosZ },
            { -sinY, sinX * cosY, cosX * cosY }
        } };


    }

    RotationMatrix Multiply(const RotationMatrix& left, const RotationMatrix& right)
    {
        RotationMatrix result{};
        for (std::size_t row = 0; row < 3; ++row)
            for (std::size_t column = 0; column < 3; ++column)
                for (std::size_t index = 0; index < 3; ++index)
                    result[row][column] += left[row][index] * right[index][column];
        return result;
    }

    Vec3 ToRotationXYZ(const RotationMatrix& matrix)
    {
        const float y = std::asin(std::clamp(-matrix[2][0], -1.0f, 1.0f));
        const float cosY = std::cos(y);
        if (std::abs(cosY) > 0.00001f)
        {
            return {
                std::atan2(matrix[2][1], matrix[2][2]),
                y,
                std::atan2(matrix[1][0], matrix[0][0])
            };
        }

        return { std::atan2(-matrix[1][2], matrix[1][1]), y, 0.0f };
    }

    Vec3 TransformVector(
        const Vec3& value,
        const RotationMatrix& matrix)
    {
        return {
            matrix[0][0] * value.x
                + matrix[0][1] * value.y
                + matrix[0][2] * value.z,

            matrix[1][0] * value.x
                + matrix[1][1] * value.y
                + matrix[1][2] * value.z,

            matrix[2][0] * value.x
                + matrix[2][1] * value.y
                + matrix[2][2] * value.z
        };
    }
}

// 親をたどってワールド座標を取得
Vec3 ActorBase::GetWorldPosition() const
{
    if (!owner_)
    {
        return position_;
    }

    const Vec3 parentPosition = owner_->GetWorldPosition();
    const Vec3 parentRotation = owner_->GetWorldRotation();

    const RotationMatrix parentMatrix =
        MakeRotationMatrix(parentRotation);

    return parentPosition
        + TransformVector(position_, parentMatrix);
}

// 親をたどってワールド回転を取得
Vec3 ActorBase::GetWorldRotation() const
{
    if (!owner_)
    {
        return GetRotation();
    }
    

    const RotationMatrix parentMatrix =
        MakeRotationMatrix(owner_->GetWorldRotation());

    const RotationMatrix localMatrix =
        MakeRotationMatrix(GetRotation());

    return ToRotationXYZ(
        Multiply(parentMatrix, localMatrix)
    );
}