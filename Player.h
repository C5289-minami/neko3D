#pragma once

#include "ActorBase.h"
#include "DrawableObject.h"
#include "PlayerInput.h"
#include "PlayerStateMachine.h"
#include "Vector3.h"
#include "Stage.h"
#include "AnimationDraw.h"
#include "PlayerAnimType.h"
#include "TransformSettings.h"
#include "Consts.h"

class Stage;

class Player : public ActorBase
{
public:
	Player();

	void Init() override;
	void Reset() override;
	void Update(float deltaTime) override;
	void Draw() const override;
    void DrawStatus() const;

    static constexpr int MaxHealth = 5;
    static constexpr float DamageRecoveryTime = 0.5f;
    int GetHealth() const { return health_; }
    bool IsDead() const { return health_ <= 0; }
    bool IsDamaging() const { return GetCurrentState() == PlayerStateType::Damaging; }
    PlayerStateType GetCurrentState() const { return stateMachine_.GetCurrentType(); }
    bool TakeDamage();

 void SetAnimation(PlayerAnimType::Type type, bool loop = true);

	// ステージとの当たり判定込みの更新
	void Update(float deltaTime, const Stage& stage, const Vec3& movementCenter = Vec3{});

	// 移動
	void SetMoveDirection(const Vec3& moveDir);
	void StopMove();

	// 回転
	void Turn(float direction, float speed, float deltaTime);
	void TurnTowards(const Vec3& moveDir, float deltaTime);
	//Playerの斜め移動
	void PlayerDiagonalMove(float deltaTime, Vec3& playerPos, const Vec3& movementCenter);

	// ジャンプ
	void JumpAction();
	void StartLungingJump(const Vec3& moveDir);
    float GetLungingMoveSpeed() const { return lungingMoveSpeed_; }
    float GetLungingJumpSpeed() const { return lungingJumpSpeed_; }
    void SetLungingMoveSpeed(float value);
    void SetLungingJumpSpeed(float value);
    void ResetLungingSpeeds();
	bool IsGrounded() const { return isGrounded_; }
	bool IsGravityEnabled() const { return gravityEnabled_; }
	void SetGravityEnabled(bool enabled);

	void SetPosition(const Vec3& position)
    {
        ActorBase::SetPosition(position);
        model_.position = position;
    }
    ObjectTransform GetTransform() const
    {
        return { model_.scale, GetPosition(), { GetPitch(), GetYaw(), model_.rotation.z } };
    }
    void SetTransform(const ObjectTransform& transform)
    {
        SetPosition(transform.position);
        SetPitch(transform.rotation.x);
        SetYaw(transform.rotation.y);
        model_.rotation = transform.rotation;
        model_.scale = transform.scale;
    }
    ModelObject& GetModelObject() { return model_; }
    const ModelObject& GetModelObject() const { return model_; }
	const PlayerInput& GetInput() const { return input_; }

    // 必殺ゲージは仮。増加タイミングは後で決める
    static constexpr float UltimateGaugeMax = 100.0f;
    float GetUltimateGauge() const { return ultimateGauge_; }
    void SetUltimateGauge(float value);
    void AddUltimateGauge(float amount);
    bool CanUseUltimate() const { return ultimateGauge_ >= UltimateGaugeMax; }
    bool TryUseUltimateGauge();

private:
    int health_{ MaxHealth };
	ModelObject model_;
	AnimationDraw animation_;
	int currentAnimIndex_{ static_cast<int>(PlayerAnimType::Idle) };
	bool currentAnimLoop_{ true };

	PlayerInput input_;
	PlayerStateMachine stateMachine_;
    float ultimateGauge_{ 0.0f };
    float lungingMoveSpeed_{ Const::PLAYER_LUNGING_MOVE_SPEED };
    float lungingJumpSpeed_{ Const::PLAYER_LUNGING_JUMP_SPEED };
	float kari_max_jissou_ = UltimateGaugeMax;

	Vec3 movementCenter_{}; // 足場の中心
	Vec3 moveInput_{};      // Xは左右、Zは前後の入力
	Vec3 velocity_{ 0.0f, 0.0f, 0.0f };
	bool isGrounded_{ false };
    bool wasTouchingPillar_{ false };
  bool gravityEnabled_{ true };
};
