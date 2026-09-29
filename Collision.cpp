// ================================
// Gameplay/Collision/Collision.cpp
// ================================
#include "Collision.h"

namespace Collision
{
	bool IsHitSphereSphere(const Vec3& centerA, float radiusA,
		const Vec3& centerB, float radiusB)
	{
		// 2つの球の中心の差を求める
		Vec3 diff = centerB - centerA;

		// 2つの球の半径を足す
		float radiusSum = radiusA + radiusB;

		// 中心間距離が半径の合計以下なら当たっている
		return diff.LengthSq() <= radiusSum * radiusSum;
	}

	bool IsHitSphereSphere(const Sphere& sphereA, const Sphere& sphereB)
	{
		// Sphereの中心座標と半径を使って判定する
		return IsHitSphereSphere(sphereA.center, sphereA.radius,
			sphereB.center, sphereB.radius);
	}
}