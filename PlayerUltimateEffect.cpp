#include "PlayerUltimateEffect.h"

#include "Boss.h"
#include "Collision.h"
#include "Consts.h"
#include "DxConv.h"
#include "EffectManager.h"
#include "Player.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "Stage.h"
#include <algorithm>
#include <cmath>

void PlayerUltimateEffect::Update(Player& player, Boss& boss, const Stage& stage, float deltaTime)
{
	if (boss.GetHealth() <= 0)
	{
		Clear(player);
		return;
	}
	if (!std::isfinite(deltaTime) || deltaTime <= 0.0f)
		return;
	// プレイヤーがウルティメイトショットを発射していない場合、残っている弾を消す。
	const auto launchPosition = player.TakeUltimateShotOrigin();
	if (!launchPosition && shots_.empty())
		return;

	const Collision::Capsule body = GetBossBody(boss, stage);
	if (body.radius <= 0.0f)
	{
		Clear(player);
		return;
	}

	if (launchPosition)
		Launch(*launchPosition, body);

	for (auto& shot : shots_)
	{
        UpdateShot(shot, player, boss, body, deltaTime);
		if (boss.GetHealth() <= 0)
		{
			Clear(player);
			return;
		}
	}

	RemoveFinishedShots();
}

void PlayerUltimateEffect::UpdateShot(
    Shot& shot, Player& player, Boss& boss, const Collision::Capsule& body, float deltaTime)
{
	const Vec3 previousPosition = shot.position;
	MoveShot(shot, deltaTime);

	if (HitsBoss(previousPosition, shot.position, body))
	{
		FinishShot(shot);
        player.CameraShakeRequest(CameraShakeType::Mixed, 0.6f, 40.0f);
		if constexpr (Const::PLAYER_ULTIMATE_DAMAGE > 0)
			boss.TakeDamage(Const::PLAYER_ULTIMATE_DAMAGE);
     boss.ApplyDownGaugeDamage(Const::BOSS_DOWN_GAUGE_MAX);
		return;
	}

	if (shot.remainingSeconds <= 0.0f)
	{
		FinishShot(shot);
		return;
	}

	UpdateEffect(shot);
}

Collision::Capsule PlayerUltimateEffect::GetBossBody(const Boss& boss, const Stage& stage)
{
	// ステージで設定した本体カプセルを、現在のボスの位置に合わせる。
	Collision::Capsule body = stage.GetCollision().GetContactCapsule();
	const Vec3 bossOffset = boss.GetWorldPosition() - stage.GetWorldPosition();
	body.start += bossOffset;
	body.end += bossOffset;
	return body;
}

void PlayerUltimateEffect::Launch(
	const Vec3& launchPosition, const Collision::Capsule& body)
{
	// 発射時に決めた速度と方向で、そのまま飛び続ける。
	const Vec3 bossCenter = (body.start + body.end) * 0.5f;
	const Vec3 direction = (bossCenter - launchPosition).Normalized();
	const Vec3 velocity = direction * Const::PLAYER_ULTIMATE_PROJECTILE_SPEED;
	// 発射位置と速度、残り寿命を記録する。
	shots_.push_back({
		launchPosition, velocity, Const::PLAYER_ULTIMATE_PROJECTILE_LIFETIME_SECONDS });
}

void PlayerUltimateEffect::MoveShot(Shot& shot, float deltaTime)
{
	// 残り寿命より先まで進まないようにする。
	const float moveSeconds = (std::min)(deltaTime, shot.remainingSeconds);
	shot.position += shot.velocity * moveSeconds;
	shot.remainingSeconds -= moveSeconds;
}

bool PlayerUltimateEffect::HitsBoss(
	const Vec3& previousPosition, const Vec3& currentPosition, const Collision::Capsule& body)
{
	// 移動区間を弾の半径で太らせ、ボス本体のカプセルと重なるか調べる。
	return DxLib::HitCheck_Capsule_Capsule(
		DxConv::ToVECTOR(previousPosition), DxConv::ToVECTOR(currentPosition),
		Const::PLAYER_ULTIMATE_PROJECTILE_RADIUS,
		DxConv::ToVECTOR(body.start), DxConv::ToVECTOR(body.end), body.radius) != 0;
}

void PlayerUltimateEffect::UpdateEffect(Shot& shot)
{
	if (EffectManager::MoveEffect(shot.effectHandle, shot.position))
		return;

	const int effect = RM().GetEffect(ResourceKeys::Effect_Kedama);
	const float yaw = std::atan2(shot.velocity.x, shot.velocity.z);
	shot.effectHandle = EffectManager::PlayEffect(effect, shot.position, yaw);
}

void PlayerUltimateEffect::FinishShot(Shot& shot)
{
	shot.remainingSeconds = 0.0f;
	EffectManager::StopEffect(shot.effectHandle);
	shot.effectHandle = -1;
}

void PlayerUltimateEffect::RemoveFinishedShots()
{
	const auto finished = std::remove_if(
		shots_.begin(), shots_.end(),
		[](const Shot& shot) { return shot.remainingSeconds <= 0.0f; });
	shots_.erase(finished, shots_.end());
}

void PlayerUltimateEffect::Clear(Player& player)
{
	player.TakeUltimateShotOrigin();
	for (const auto& shot : shots_)
		EffectManager::StopEffect(shot.effectHandle);
	shots_.clear();
}
