// =============================
// Gameplay/Physics/Raycast.h
// =============================
#pragma once
#include "Vector3.h"
#include <vector>

namespace Physics
{
	struct RayHit
	{
		Vec3 point{};			// 光線が衝突した位置
		Vec3 normal{};			// 衝突した面の法線
		float distance{ 0.0f };	// 光線の開始位置から衝突位置までの距離（初期値 0.0f）
	};

    // モデル1個に対する、元のレイキャスト処理。
    bool Raycast(int modelHandle, Vec3 start, Vec3 end, RayHit& hit);

    // 元の処理を各モデルに対して呼び、最も近い交点を返す。
    bool Raycast(const std::vector<int>& modelHandles, Vec3 start, Vec3 end, RayHit& hit);
    bool RaycastDown(const std::vector<int>& modelHandles, Vec3 origin, float maxDistance, RayHit& hit);
    bool RaycastUp(const std::vector<int>& modelHandles, Vec3 origin, float maxDistance, RayHit& hit);
}
