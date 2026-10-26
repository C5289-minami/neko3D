// =============================
// Core/Math/DxConv.h
// =============================
#pragma once
#include "DxLib.h"
#include "Vector3.h"
#include "Matrix.h"

namespace DxConv
{
    inline DxLib::VECTOR ToVECTOR(const Vec3& v) noexcept
    {
        return VGet(v.x, v.y, v.z);
    }

    inline Vec3 ToVec3(const DxLib::VECTOR& v) noexcept
    {
        return Vec3{ v.x, v.y, v.z };
    }

    inline DxLib::MATRIX ToMATRIX(const Matrix4& m) noexcept
    {
        DxLib::MATRIX result;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                result.m[i][j] = m.m[i][j];
            }
        }
        return result;
	}
}
