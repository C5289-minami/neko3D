#pragma once
#include "PlayerBaseState.h"
#include "Vector3.h"

class PlayerAttackState : public PlayerBaseState
{
public:
    PlayerAttackState(PlayerStateMachine& stateMachine, Player& player);
    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
	void Draw() const;

private:
	bool ballActive_{};
	Vec3 ballPosition_{};
	Vec3 ballVelocity_{};
	float elapsed_{};
};
