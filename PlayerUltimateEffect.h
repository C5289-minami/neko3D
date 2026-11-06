#pragma once

#include "Vector3.h"
#include <vector>

class Boss;
class Player;
class Stage;

namespace Collision
{
	struct Capsule;
}

// 必殺技の弾を、命中するか寿命が尽きるまで管理する。
class PlayerUltimateEffect
{
public:
	void Update(Player& player, Boss& boss, const Stage& stage, float deltaTime);
	void Clear(Player& player);

private:
	struct Shot
	{
		Vec3 position;
		Vec3 velocity;
		float remainingSeconds;
		int effectHandle{ -1 };
	};

	static Collision::Capsule GetBossBody(const Boss& boss, const Stage& stage);
	void Launch(const Vec3& launchPosition, const Collision::Capsule& body);
	void UpdateShot(Shot& shot, Boss& boss, const Collision::Capsule& body, float deltaTime);
	static void MoveShot(Shot& shot, float deltaTime);
	static bool HitsBoss(
		const Vec3& previousPosition, const Vec3& currentPosition, const Collision::Capsule& body);
	static void UpdateEffect(Shot& shot);
	static void FinishShot(Shot& shot);
	void RemoveFinishedShots();

	std::vector<Shot> shots_;
};
