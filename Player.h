#pragma once

#include "ActorBase.h"
#include "DrawableObject.h"
#include "PlayerInput.h"
#include "PlayerStateMachine.h"
#include "Vector3.h"
// #include "Stage.h"
#include "ModelCollision.h"
#include "ModelAnimation.h"
#include "PlayerAnimType.h"
#include "ObjectTransform.h"
#include "Consts.h"
#include "CharSettings.h"
#include "AnimationHandle.h"

// class Stage;

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

	// // ステージとの当たり判定込みの更新
	// void Update(float deltaTime, const Stage& stage, const Vec3& movementCenter = Vec3{});
	// シーンが登録したモデルとの当たり判定込みの更新
	void Update(float deltaTime, ModelCollision& collision, const Vec3& movementCenter = Vec3{});

	// 移動
	void SetMoveDirection(const Vec3& moveDir);
	void StopMove();
	void SetMoveRate(float rate) { moveRate_ = rate; }
	void SetVerticalSpeed(float speed) { velocity_.y = speed; }
	bool IsTouchingWall() const { return touchingWall_; }
	const Vec3& GetWallNormal() const { return wallNormal_; }
	float GetWallTopY() const { return wallTopY_; }
	bool IsNearWallTop() const//
	{
		const float distance = wallTopY_ - GetPosition().y;
		return touchingWall_ && distance >= -Const::PLAYER_SKIN &&
			distance <= Const::PLAYER_CLIMB_TOP_HOLD_DISTANCE;
	}
	Vec3 GetWorldMoveDirection(const Vec3& moveDir) const;
	void SetDefMove(bool defMove) { isdefMove_ = defMove; }

#ifdef _DEBUG//ShitのDebug用のダッシュ機能のセッ
	void SetDebugDashEnabled(bool enabled) { debugDashEnabled_ = enabled; }
#endif

	// 回転
	void Turn(float direction, float speed, float deltaTime);
	void TurnTowards(const Vec3& moveDir, float deltaTime);

	// ジャンプ
	static constexpr float CoyoteTime = 0.1f;
	bool CanGroundJump() const { return gravityEnabled_ && (isGrounded_ || coyoteTimeRemaining_ > 0.0f); }
	// 地上・踏み外し直後は通常ジャンプ、それ以外の空中では二段ジャンプ。
	bool TryJump();
	bool JumpAction();

	void StartDoubleJump();
	bool CanDoubleJump() const { return gravityEnabled_ && !isGrounded_ && !doubleJumpUsed_; }
	float GetDoubleJumpSpeed() const { return doubleJumpSpeed_; }
	void SetDoubleJumpSpeed(float value);
	void ResetDoubleJumpSpeed();
	void StartWallJump();


    // 壁登り・滑り落ち・壁キックは、それぞれ専用の速度を持つ。
    float GetClimbingSpeed() const { return climbingSpeed_; }
    float GetWallSlideSpeed() const { return wallSlideSpeed_; }
    void SetClimbingSpeed(float value);
    void SetWallSlideSpeed(float value);
    void ResetClimbingSpeeds();

    //壁のぼり
    bool IsPillarElevatorActive() const { return elevatorPillar_ != nullptr; }
    void StartPillarElevator(const ModelObject* pillar, const Vec3& center, float topY);
    bool UpdatePillarElevator(float deltaTime);
    void EndPillarElevator();
    void StartPillarExitJump();


	//ゲッター・セッター
	float GetWallJumpMoveSpeed() const { return wallJumpMoveSpeed_; }
	float GetWallJumpSpeed() const { return wallJumpSpeed_; }
	Vec3& GetVelocity() { return velocity_; }
	void SetWallJumpMoveSpeed(float value);
	void SetWallJumpSpeed(float value);
	void ResetWallJumpSpeeds();
	bool IsGrounded() const { return isGrounded_; }
	void SetGround(bool grounded) { isGrounded_ = grounded; }
	bool IsGravityEnabled() const { return gravityEnabled_; }
	void SetGravityEnabled(bool enabled);

	void SetPosition(const Vec3& position)
	{
		ActorBase::SetPosition(position);
	}
    // 攻撃球の出現位置。攻撃処理とGameTestSceneのデバッグ表示で共通して使う。
    Vec3 GetAttackBallSpawnPosition() const;
    // 開始場所を変えるときは、Rで戻る場所も一緒に変える。
    void SetStartPosition(const Vec3& position);
    void ResetPosition();
	ObjectTransform GetTransform() const
	{
		return { model_.GetScale(), GetPosition(), GetRotation() };
	}
	void SetTransform(const ObjectTransform& transform)
	{
		SetPosition(transform.position);
		SetPitch(transform.rotation.x);
		SetYaw(transform.rotation.y);
		model_.SetScale(transform.scale);
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

	// キャラクター設定
	CharData& GetCharData() { return charData; }
	const CharData& GetCharData() const{ return charData; }
	void SetCharData(const CharData& data) { charData = data; }
	void SetModelKey(const std::wstring& key);

	PlayerStateMachine& GetStateMachine() { return stateMachine_; }
    const PlayerStateMachine& GetStateMachine() const { return stateMachine_; }

	void SetAnimation(
		PlayerAnimType::Type type,
		bool loop = true,
		bool playAfterBlend = true,
		float blendSeconds = 0.15f
	);
	AnimationHandle GetAnimationHandle(PlayerAnimType::Type type) const;
	ModelAnimation& GetAnimation() { return animation_; }
	void UpdateAnimation(float deltaTime);

	Vec3 GetPillarElevatorCenter() const { return elevatorCenter_; }
private:
    void UpdateMovement(float deltaTime);
	// 横速度を決めた後、MoveでXYZの速度をまとめて座標に反映する。
	void UpdateHorizontalVelocity(float deltaTime);//nyuuryokukei
	void Move(float deltaTime);

	// ステージ中心を基準に、円周移動・前後移動の横速度を設定する。
	void UpdateStageMoveVelocity(float moveSpeed, float deltaTime);

	int health_{ MaxHealth };
	ModelObject model_;

	PlayerInput input_;
	PlayerStateMachine stateMachine_;
	float ultimateGauge_{ 0.0f };
	// float lungingMoveSpeed_{ Const::PLAYER_LUNGING_MOVE_SPEED };
	float doubleJumpSpeed_{ Const::PLAYER_DOUBLE_JUMP_SPEED };
	// float climbingSpeed_{ Const::PLAYER_CLIMBING_SPEED };
	float climbingSpeed_{ Const::PLAYER_ELEVATOR_SPEED };
	float wallSlideSpeed_{ Const::PLAYER_WALL_SLIDE_SPEED };
	float wallJumpMoveSpeed_{ Const::PLAYER_WALL_JUMP_MOVE_SPEED };
	float wallJumpSpeed_{ Const::PLAYER_WALL_JUMP_SPEED };
	float kari_max_jissou_ = UltimateGaugeMax;

    Vec3 startPosition_{}; // Rで戻る最初の場所。
	Vec3 movementCenter_{}; // 足場の中心
	Vec3 moveInput_{};      // Xは左右、Zは前後の入力
	Vec3 velocity_{ 0.0f, 0.0f, 0.0f }; // 当たり判定で補正する前の移動速度（ワールド座標）
	float moveRate_{ 1.0f }; // 移動速度の倍率

    //const float Wall_MARGIN = Const::PLAYER_RADIUS + Const::PLAYER_HEIGHT;
    //おれらにはModelCollisionのやつがあることを思い出した
    // bool touchingPillar = false;
    bool touchingModel_ = false;
	bool isGrounded_{ false };
	// bool wasTouchingPillar_{ false };
	bool wasTouchingModel_{ false };
	bool gravityEnabled_{ true };

	// false：ダブルジャンプをまだ使っていない。true：使用済み。
	// 着地するまでtrueを保ち、Spaceを連打しても3回目は出せないようにする。
	bool doubleJumpUsed_{ false };
	float coyoteTimeRemaining_{ 0.0f };

	bool touchingWall_{ false };
	Vec3 wallNormal_{};
	bool isdefMove_{ false };// デフォルトの移動をするかどうか
	float wallTopY_{ 0.0f };
	const ModelObject* elevatorPillar_{ nullptr };
	Vec3 elevatorCenter_{};
	float elevatorTopY_{};
	bool elevatorInputConsumed_{ false };

#ifdef _DEBUG
	bool debugDashEnabled_{ false };
#endif
	// キャラクター設定
	CharData charData;

	// Animations
	ModelAnimation animation_;

	PlayerAnimType::Type currentAnimType_{ PlayerAnimType::Idle };
	bool currentAnimLoop_{ true };
	AnimationHandle idleAnimHandle_;
	AnimationHandle walkAnimHandle_;
	AnimationHandle jumpAnimHandle_;
	AnimationHandle doubleJumpAnimHandle_;
};
