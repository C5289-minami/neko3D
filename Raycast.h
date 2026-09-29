// =============================
// Gameplay/Physics/Raycast.h
// =============================
#pragma once
#include "Vector3.h"

namespace Physics
{
	struct RayHit
	{
		Vec3 point{};			// 光線が衝突した位置
		Vec3 normal{};			// 衝突した面の法線
		float distance{ 0.0f };	// 光線の開始位置から衝突位置までの距離（初期値 0.0f）
	};

	bool Raycast(int modelHandle, Vec3 start, Vec3 end, RayHit& hit);
	bool RaycastDown(int modelHandle, Vec3 origin, float maxDistance, RayHit& hit);
	bool RaycastUp(int modelHandle, Vec3 origin, float maxDistance, RayHit& hit);
}
