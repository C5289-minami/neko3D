#pragma once

#include "ActorBase.h"
#include "DrawableObject.h"
#include "PlayerInput.h"
#include "PlayerStateMachine.h"
#include "Vector3.h"
#include "Stage.h"
#include "AnimationDraw.h"
#include "PlayerAnimType.h"

class Stage;

class Player : public ActorBase
{
public:
	Player();

	void Init() override;
	void Reset() override;
	void Update(float deltaTime) override;
	void Draw() const override;

 void SetAnimation(PlayerAnimType::Type type, bool loop = true);

	// ステージとの当たり判定込みの更新
	void Update(float deltaTime, const Stage& stage);

	// 移動
	void SetMoveDirection(const Vec3& moveDir, float speed);
	void StopMove();

	// 回転
	void Turn(float direction, float speed, float deltaTime);
	void TurnTowards(const Vec3& moveDir, float deltaTime);

	// ジャンプ
	void JumpAction();

	const ModelObject& GetModelObject() const { return model_; }
	const PlayerInput& GetInput() const { return input_; }

private:
	ModelObject model_;
	AnimationDraw animation_;
	int currentAnimIndex_{ static_cast<int>(PlayerAnimType::Idle) };
	bool currentAnimLoop_{ true };

	PlayerInput input_;
	PlayerStateMachine stateMachine_;

	Vec3 velocity_{ 0.0f, 0.0f, 0.0f };
	bool isGrounded_{ false };
};
