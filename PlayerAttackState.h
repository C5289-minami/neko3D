#pragma once
#include "PlayerBaseState.h"
#include "Collision.h"

class PlayerAttackState : public PlayerBaseState
{
public:
    PlayerAttackState(PlayerStateMachine& stateMachine, Player& player);
    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
	void Draw() const;
	const Collision::Sphere& GetAttackSphere() const { return attackSphere_; }

private:
	bool ballActive_{};
	Collision::Sphere attackSphere_{};
	Vec3 ballVelocity_{};
	float elapsed_{};
};
