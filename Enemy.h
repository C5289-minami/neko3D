// =============================
// Gameplay/Actors/Enemy.h
// =============================
#pragma once
#include "Vector3.h"
#include "Collision.h"

class Enemy
{
public:
    Enemy() = default;
    const Vec3& GetPosition() const { return position; }
    void SetPosition(const Vec3& pos) { position = pos; }
    float GetRadius() const { return radius; }
    float GetHeight() const { return height; }

    bool IsAlive() const { return isAlive; }
    int GetHp() const { return hp; }
    int GetMaxHp() const { return maxHp; }

    float GetHpRate() const;
	float GetYaw() const { return yaw; }

    Vec3 GetHitCenter() const { return position + Vec3::Up() * 50.0f; }
    float GetHitRadius() const { return 70.0f; }
    Collision::Sphere GetHitSphere() const { return Collision::Sphere{ GetHitCenter(), radius }; }

    void Init();
    void Reset(const Vec3& startPosition, float startYaw);
    void Update(float deltaTime, const Vec3& playerPos);
    void Draw() const;
    void DrawDebug() const;
    void Release();

    void TakeDamage(int damage);
    void Kill();
    void ApplyKnockback(const Vec3& direction, float speed);

private:
    void StartAnimation(int nextAnim);

    static constexpr int ANIM_IDLE = 0;
    static constexpr int ANIM_MOVE = 1;

    int modelHandle = -1;
    Vec3 position{ 0.0f, 0.0f, 0.0f };
    Vec3 scale{ 100.0f, 100.0f, 100.0f };
    float yaw{ 0.0f };

    // ---- animation ----
    int animAttachIndex{ -1 };          // モデルに取り付けたアニメーションの管理番号
    float animTime{ 0.0f };             // 現在の再生時間
    float animTotalTime{ 0.0f };        // アニメーション全体の長さ
    float animSpeed{ 30.0f };           // アニメーションの再生速度
    int currentAnim{ -1 };              // 現在再生中のアニメーション番号

    // ---- move ----
    float moveSpeed{ 80.0f };           // スライムが移動する速さ
    float stopDistance{ 120.0f };       // プレイヤーに近づきすぎないように停止する距離
    float radius{ 60.0f };              // スライムの半径
    float height{ 100.0f };             // スライムの高さ

    int maxHp{ 20 };                    // 最大HP
    int hp{ 20 };                       // 現在のHP
    bool isAlive{ true };               // 生存していればtrue

    Vec3 knockbackVelocity{};           // ノックバック速度
    float knockbackFriction{ 1000.0f }; // ノックバック時の摩擦力（減速力）
    float knockbackStopSpeed{ 20.0f };  // ノックバックを止める際の速さ
};
