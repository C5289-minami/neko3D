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
namespace Collision
{
    bool IsHitEllipsoidEllipsoid(const Ellipsoid& a, const Ellipsoid& b)
    {

		// 2つの楕円体の半径が0以下だったら当たり判定を行わない
		if (a.radii.x <= 0.0f || a.radii.y <= 0.0f || a.radii.z <= 0.0f ||
            b.radii.x <= 0.0f || b.radii.y <= 0.0f || b.radii.z <= 0.0f)
        {
            return false;
        }

        const Vec3 difference = b.center - a.center;
        const Vec3 radiiSum = a.radii + b.radii;
        const Vec3 normalized{
            difference.x / radiiSum.x,
            difference.y / radiiSum.y,
            difference.z / radiiSum.z
        };
        return normalized.LengthSq() <= 1.0f;
    }


}