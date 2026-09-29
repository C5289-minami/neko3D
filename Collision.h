// ==============================
// Gameplay/Collision/Collision.h
// ==============================
#pragma once
#include "Vector3.h"

namespace Collision									// ‚ ‚½‚è”»’è—p–¼‘O‹óŠÔ
{
	struct Sphere									// ‹…‚Ì‚ ‚½‚è”»’èî•ñ
	{
		Vec3 center{};								// ’†SÀ•W
		float radius{};							// ”¼Œa
	};

	bool IsHitSphereSphere(							// ‹…“¯m‚Ì‚ ‚½‚è”»’è
		const Vec3& centerA, float radiusA,
		const Vec3& centerB, float radiusB);

	bool IsHitSphereSphere(							// ‹…“¯m‚Ì‚ ‚½‚è”»’è
		const Sphere& sphereA, const Sphere& sphereB);
}