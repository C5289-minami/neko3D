#pragma once
#include "Vector3.h"
#include "PlayerStateMachine.h"
#include "Stage.h"
#include "PlayerInput.h"

class Player
{
public:
	Player();

	void Init();
	void Reset();
	void Update(float deltaTime,const Stage& stage);
	void Draw() const;

	// ゲッターセッター
	const PlayerInput& GetInput() const { return input_; }
	void SetMoveDirection(const Vec3& localDir, float speed); 
	void StopMove();
	void Turn(float direction, float speed, float deltaTime);
private:
	int modelHandle{ -1 };
	Vec3 scale_{ 100.0f,100.0f,100.0f };
	Vec3 position_{0.0f,0.0f,0.0f};
	Vec3 velocity_{0.0f,0.0f,0.0f};
	float yaw_{ 0.0f };
	bool isGrounded_{ true };

	PlayerStateMachine stateMachine_;
	PlayerInput input_;
};

