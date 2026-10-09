// ==============================
// Gameplay/Collision/Collision.h
// ==============================
#pragma once
#include "Vector3.h"

struct ModelObject;

namespace Collision									// あたり判定用名前空間
{
	//Axisの方向に沿った楕円体で、モデルのワールド空間の境界に合わせてフィットさせる。
    struct Ellipsoid
    {
        Vec3 center{};
        Vec3 radii{};
    };

	//近似値だけで、各軸の半径の合計を使用して判定する。
    bool IsHitEllipsoidEllipsoid(const Ellipsoid& a, const Ellipsoid& b);

	//モデルのコンタクトを更新し、接触している間は両方のモデルをハイライト表示する。
    void UpdateModelContact(ModelObject& a, ModelObject& b);

	struct Sphere									// 球のあたり判定情報
	{
		Vec3 center{};								// 中心座標
		float radius{};							// 半径
	};

	bool IsHitSphereSphere(							// 球同士のあたり判定
		const Vec3& centerA, float radiusA,
		const Vec3& centerB, float radiusB);

	bool IsHitSphereSphere(							// 球同士のあたり判定
		const Sphere& sphereA, const Sphere& sphereB);
}