#pragma once
#include "Vector3.h"

struct Matrix4
{
	float m[4][4]{};

	static constexpr
		[[nodiscard]] Matrix4 CreateIdentity()noexcept
	{
		Matrix4 result;
		result.m[0][0] = 1.0f;
		result.m[1][1] = 1.0f;
		result.m[2][2] = 1.0f;
		result.m[3][3] = 1.0f;
		return result;
	}

	static constexpr
		[[nodiscard]] Matrix4 CreateTranslation(Vec3 pos)noexcept
	{
		Matrix4 result = CreateIdentity();
		result.m[3][0] = pos.x;
		result.m[3][1] = pos.y;
		result.m[3][2] = pos.z;
		return result;
	}

	static constexpr
		[[nodiscard]] Matrix4 CreateScale(Vec3 scale)noexcept
	{
		Matrix4 result = CreateIdentity();
		result.m[0][0] = scale.x;
		result.m[1][1] = scale.y;
		result.m[2][2] = scale.z;
		return result;
	}


	static
		[[nodiscard]] Matrix4 CreateRotationX(float angle)noexcept
	{
		Matrix4 result = CreateIdentity();
		result.m[1][1] = std::cos(angle);
		result.m[1][2] = std::sin(angle);
		result.m[2][1] = -std::sin(angle);
		result.m[2][2] = std::cos(angle);
		return result;
	}

	static
		[[nodiscard]] Matrix4 CreateRotationY(float angle)noexcept
	{
		Matrix4 result = CreateIdentity();
		result.m[0][0] = std::cos(angle);
		result.m[0][2] = -std::sin(angle);
		result.m[2][0] = std::sin(angle);
		result.m[2][2] = std::cos(angle);
		return result;
	}

	static
		[[nodiscard]] Matrix4 CreateRotationZ(float angle)noexcept
	{
		Matrix4 result = CreateIdentity();
		result.m[0][0] = std::cos(angle);
		result.m[0][1] = std::sin(angle);
		result.m[1][0] = -std::sin(angle);
		result.m[1][1] = std::cos(angle);
		return result;
	}



	Matrix4 operator* (const Matrix4 other) const
	{
		Matrix4 result{};
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				for (int k = 0; k < 4; ++k)
				{
					result.m[row][col]
						+= m[row][k] * other.m[k][col];
				}
			}
		}
		return result;
	}

	Matrix4& operator*= (const Matrix4 other)
	{
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				for (int k = 0; k < 4; ++k)
				{
					m[row][col]
						+= m[row][k] * other.m[k][col];
				}
			}
		}
		return *this;
	}
};