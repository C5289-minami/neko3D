// =============================
// Gameplay/Actors/Bullet.cpp
// =============================
#include "Bullet.h"
#include "DxLib.h"
#include "DxConv.h"
#include "Consts.h"

void Bullet::Reset()
{
	// 弾を未使用状態に戻す
	isActive = false;
	position = {};
	velocity = {};
	radius = 20.0f;
	lifeTime = 0.0f;
	maxLifeTime = 2.0f;

	// ダメージを初期化
	damage = 5;

	// ホーミング用情報を初期化
	type = Type::Normal;
	targetEnemyIndex = -1;
	speed = 900.0f;
}

void Bullet::Update(float deltaTime)
{
	// 有効でない弾は更新しない
	if (!isActive) return;

	// 速度に応じて移動する
	position += velocity * deltaTime;

	// 寿命を進める
	lifeTime += deltaTime;
	if (lifeTime >= maxLifeTime)
	{
		isActive = false;
		return;
	}
}

void Bullet::UpdateHoming(float deltaTime, const Vec3& targetPosition, float turnPower)
{
	// 有効でない弾は更新しない
	if (!IsActive()) return;

	// 弾からターゲットへ向かうベクトルを作る
	Vec3 toTarget = targetPosition - position;

	// 現在方向とターゲット方向を混ぜて、ゆるやかに曲げる
	if (toTarget.LengthSq() > 0.0001f &&
		velocity.LengthSq() > 0.0001f)
	{
		Vec3 currentDir = velocity.Normalized();
		Vec3 targetDir = toTarget.Normalized();

		float t = turnPower * deltaTime;
		t = (std::min)(t, 1.0f);

		Vec3 newDir = currentDir * (1.0f - t) + targetDir * t;

		if (newDir.LengthSq() > 0.0001f)
		{
			velocity = newDir.Normalized() * speed;
		}
	}

	// 移動と寿命を更新する
	Update(deltaTime);
}

void Bullet::Draw() const
{
	// 有効でない弾は描画しない
	if (!isActive) return;

	// 通常弾は球で描画する
	if (type == Type::Normal)
	{
		DxLib::DrawSphere3D(DxConv::ToVECTOR(position), radius,
			16, GetColor(0, 255, 255), GetColor(0, 128, 255), true);
		return;
	}

	// ホーミング弾は円すいで描画する
	DrawHoming();
}

void Bullet::DrawDebug() const
{
	// 有効でない弾は描画しない
	if (!isActive) return;

	// あたり判定用の球を表示する
	DxLib::DrawSphere3D(DxConv::ToVECTOR(position), radius,
		12, GetColor(255, 255, 0), GetColor(255, 255, 0), false);
}

void Bullet::DrawHoming() const
{
	// 速度から進行方向を作る
	Vec3 direction = velocity;

	if (direction.LengthSq() < Const::EPS * Const::EPS)
	{
		direction = { 0.0f, 0.0f, 1.0f };
	}
	else
	{
		direction = direction.Normalized();
	}

	// 円すいの先端と根元の位置を作る
	Vec3 tipPosition = position + direction * 45.0f;
	Vec3 basePosition = position + direction * -25.0f;

	// 進行方向を向いた円すいを描画する
	DrawCone3D(
		DxConv::ToVECTOR(tipPosition),
		DxConv::ToVECTOR(basePosition),
		18.0f, 16,
		DxLib::GetColor(255, 160, 0),
		DxLib::GetColor(255, 255, 0),
		true);
}

void Bullet::Fire(const Vec3& startPosition, const Vec3& direction, float bulletSpeed)
{
	// 通常弾として設定する
	type = Type::Normal;
	speed = bulletSpeed;
	targetEnemyIndex = -1;

	// 発射位置と速度を設定する
	position = startPosition;
	velocity = direction.Normalized() * speed;
	lifeTime = 0.0f;
	isActive = true;
}

void Bullet::FireHoming(const Vec3& startPosition, const Vec3& direction, float bulletSpeed, int targetIndex)
{
	// ホーミング弾として設定する
	type = Type::Homing;
	speed = bulletSpeed;
	targetEnemyIndex = targetIndex;

	// 発射位置と速度を設定する
	position = startPosition;
	velocity = direction.Normalized() * speed;
	lifeTime = 0.0f;
	isActive = true;
}