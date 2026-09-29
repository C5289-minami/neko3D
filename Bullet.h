// =============================
// Gameplay/Actors/Bullet.h
// =============================
#pragma once
#include "Vector3.h"
#include "Collision.h"

class Bullet
{
public:
	enum class Type					// 弾の種類
	{
		Normal,						// 通常弾
		Homing,						// ホーミング弾
	};

	Bullet() = default;				// コンストラクタ

	// accessor
	bool IsActive() const { return isActive; }										// 有効状態を取得
	void Deactivate() { isActive = false; }										// 弾を無効化
	const Vec3& GetPosition() const { return position; }							// 位置を取得
	float GetRadius() const { return radius; }									// 半径を取得

	int GetDamage() const { return damage; }										// ダメージを取得
	Collision::Sphere GetHitSphere() const { return Collision::Sphere{ position, radius }; }	// 命中判定用Sphereを取得

	Type GetType() const { return type; }										// 弾の種類を取得
	bool IsHoming() const { return (type == Type::Homing); }						// ホーミング弾か調べる
	int GetTargetEnemyIndex() const { return targetEnemyIndex; }					// ターゲット番号を取得

	// lifecycle
	void Reset();																// 初期状態に戻す
	void Update(float deltaTime);												// 弾を更新
	void UpdateHoming(float deltaTime, const Vec3& targetPosition, float turnPower);	// ホーミング弾を更新
	void Draw() const;															// 弾を描画
	void DrawDebug() const;														// 判定球を表示
	void DrawHoming() const;														// ホーミング弾を描画

	void Fire(const Vec3& startPosition, const Vec3& direction, float bulletSpeed);			// 通常弾を発射
	void FireHoming(const Vec3& startPosition, const Vec3& direction, float bulletSpeed, int targetIndex);	// ホーミング弾を発射

private:
	bool isActive{ false };			// 有効状態
	Vec3 position{};					// 位置
	Vec3 velocity{};					// 速度
	float radius{ 20.0f };			// 判定半径
	float lifeTime{ 0.0f };			// 経過時間
	float maxLifeTime{ 2.0f };		// 寿命

	int damage{ 5 };					// ダメージ

	// Homing
	Type type{ Type::Normal };		// 弾の種類
	int targetEnemyIndex{ -1 };		// ターゲット番号
	float speed{ 900.0f };			// 弾の速さ
};
