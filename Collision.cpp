// ================================
// Gameplay/Collision/Collision.cpp
// ================================
#include "Collision.h"
#include "DxConv.h"
#include "DxLib.h"
#include <cmath>

namespace Collision
{
	bool IsHitSphereSphere(const Vec3& centerA, float radiusA,
		const Vec3& centerB, float radiusB)
	{
		// 2‚Â‚Ì‹…‚Ì’†S‚Ì·‚ğ‹‚ß‚é
		Vec3 diff = centerB - centerA;

		// 2‚Â‚Ì‹…‚Ì”¼Œa‚ğ‘«‚·
		float radiusSum = radiusA + radiusB;

		// ’†SŠÔ‹——£‚ª”¼Œa‚Ì‡ŒvˆÈ‰º‚È‚ç“–‚½‚Á‚Ä‚¢‚é
		return diff.LengthSq() <= radiusSum * radiusSum;
	}

	bool IsHitSphereSphere(const Sphere& sphereA, const Sphere& sphereB)
	{
		// Sphere‚Ì’†SÀ•W‚Æ”¼Œa‚ğg‚Á‚Ä”»’è‚·‚é
		return IsHitSphereSphere(sphereA.center, sphereA.radius,
			sphereB.center, sphereB.radius);
	}
}
namespace Collision
{
    bool IsHitEllipsoidEllipsoid(const Ellipsoid& a, const Ellipsoid& b)
    {

		// 2‚Â‚Ì‘È‰~‘Ì‚Ì”¼Œa‚ª0ˆÈ‰º‚¾‚Á‚½‚ç“–‚½‚è”»’è‚ğs‚í‚È‚¢
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
namespace Collision
{
    void DrawDebugEllipsoid(const Ellipsoid& e)
    {
#ifndef NDEBUG
        if (!showDebugEllipsoids || e.radii.x <= 0 || e.radii.y <= 0 || e.radii.z <= 0) return;
        constexpr int segments = 48;
        constexpr float tau = 6.28318530718f;
        const unsigned int color = GetColor(0, 255, 80);
        // ÚG‚Ì‹ß—”»’è‚Æ“¯‚¶ƒ[ƒ‹ƒh²‚É‰ˆ‚Á‚Ä‘È‰~‘Ì‚ğ•`‰æ‚·‚éB
        for (int ring = 0; ring < 15; ++ring)
        {
            const auto point = [&](int step) {
                const float t = tau * step / segments;
                if (ring < 7)
                {
                    const float phi = (ring - 3) * tau / 16;
                    return e.center + Vec3{e.radii.x * std::cos(phi) * std::cos(t),
                        e.radii.y * std::sin(phi), e.radii.z * std::cos(phi) * std::sin(t)};
                }
                const float theta = tau * (ring - 7) / 8;
                return e.center + Vec3{e.radii.x * std::cos(t) * std::cos(theta),
                    e.radii.y * std::sin(t), e.radii.z * std::cos(t) * std::sin(theta)};
            };
            for (int i = 0; i < segments; ++i)
                DrawLine3D(DxConv::ToVECTOR(point(i)), DxConv::ToVECTOR(point(i + 1)), color);
        }
#endif
    }
}
