#pragma once

#include "EntityBase.h"
#include "PlayerInput.h"
#include "PlayerStateMachine.h"
#include "Vector3.h"
#include "Stage.h"

class Player : public EntityBase
{
public:
	Player();

	void Init() override;
	void Reset() override;
	void Update(float deltaTime) override;
	void Update(float deltaTime, const Stage& stage);
	void Draw() const override;

	const PlayerInput& GetInput() const { return input_; }
	void SetMoveDirection(const Vec3& moveDir, float speed);
	void StopMove();
	void Turn(float direction, float speed, float deltaTime);
	void TurnTowards(const Vec3& moveDir, float deltaTime);
	void JumpAction();

private:
	int modelHandle{ -1 };
	Vec3 scale_{ 100.0f, 100.0f, 100.0f };
	Vec3 velocity_{ 0.0f, 0.0f, 0.0f };
	bool isGrounded_{ true };

	PlayerStateMachine stateMachine_;
	PlayerInput input_;
};

